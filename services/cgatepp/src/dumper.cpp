
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <cgatepp/dumper.hpp>
#include <cgatepp/exception.hpp>
#include "dumper.pb.h"


namespace cgatepp
{

//===============================================================================
Dumper::Dumper(std::string file_path, size_t file_size_, Listener::IHandler* wrapped_handler)
    : file_path_(file_path), file_size_(file_size_), wrapped_handler_(wrapped_handler), state_(State::DUMPING)
{
    auto ids_list = get_file_ids_list();
    current_id_ = ids_list.empty() ? 0 : ids_list.back();

    mapped_file_params_.path = compose_file_path(current_id_);
    mapped_file_params_.flags = boost::iostreams::mapped_file::readwrite;
    mapped_file_params_.offset = 0;

    if(ids_list.empty())
    {
        mapped_file_params_.new_file_size = file_size_;  //this means to create new file
    }
    else
    {
        size_t fs_file_size = boost::filesystem::file_size(mapped_file_params_.path);
        if(fs_file_size != file_size_)
        {
            throw DumperException(std::string("Memory-mapped file size mismatch! On filesystem: ") + std::to_string(fs_file_size) + std::string(" bytes; Requested: ") + std::to_string(file_size_));
        }
    }

    mapped_file_.open(mapped_file_params_);
    if(!mapped_file_.is_open())
    {
        throw DumperException("Can not open memory mapped file");
    }

    data_ = mapped_file_.data();
    current_position_ptr_ = reinterpret_cast<uint64_t*>(data_+0);

    uint64_t pos = get_pos();
    if(pos == 0)
    {
        set_pos(pos + sizeof(uint64_t));
    }

    dumper::Record record;
    record.set_record_type(dumper::Record::DUMPER_OPEN);
    record.set_timestamp(get_time());
    write_next(&record);
}


//===============================================================================
Dumper::~Dumper()
{
    if(mapped_file_.is_open())
    {

        dumper::Record record;
        record.set_record_type(dumper::Record::DUMPER_CLOSE);
        record.set_timestamp(get_time());
        write_next(&record);

        mapped_file_.close();
    }
}


//===============================================================================
void Dumper::replay_recorded_stream(bool wait_by_timestamps)
{
    if(wrapped_handler_ == nullptr)
    {
        return;
    }

    state_ = State::REPLAYING;

    uint64_t pos = open_file_and_get_pos(get_file_ids_list().front(), 0);

    uint64_t prev_timestamp = 0;
    uint64_t new_timestamp = 0;

    dumper::Record* record = reinterpret_cast<dumper::Record*>(read_next(&pos));
    while(record != nullptr)
    {
        new_timestamp = record->timestamp();
        if(prev_timestamp == 0)
        {
            prev_timestamp = new_timestamp;
        }

        dumper::Record_Type record_type = record->record_type();
        if(wait_by_timestamps && record_type != dumper::Record::DUMPER_OPEN && record_type != dumper::Record::DUMPER_CLOSE)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(new_timestamp - prev_timestamp));
        }

        switch(record_type)
        {
            case dumper::Record::LISTENER_OPEN:
                wrapped_handler_->on_listener_open();
                break;

            case dumper::Record::LISTENER_CLOSE:
                wrapped_handler_->on_listener_close();
                break;

            case dumper::Record::TRANSACTION_BEGIN:
                wrapped_handler_->on_transaction_begin();
                break;

            case dumper::Record::TRANSACTION_COMMIT:
                wrapped_handler_->on_transaction_commit();
                break;

            case dumper::Record::STREAM_DATA:
            {
                cg_msg_streamdata_t msg;
                const dumper::StreamData& stream_data = record->stream_data();
                msg.type = CG_MSG_STREAM_DATA;
                msg.data_size = stream_data.data().size();
                msg.data = (void*)stream_data.data().c_str();
                msg.msg_index = stream_data.msg_index();
                msg.msg_id = stream_data.msg_id();
                msg.msg_name = stream_data.msg_name().c_str();
                msg.rev = stream_data.rev();
                msg.num_nulls = stream_data.num_nulls();
                msg.nulls = reinterpret_cast<const uint8_t*>(stream_data.nulls().c_str());

                wrapped_handler_->on_stream_data(&msg);
                break;
            }

            case dumper::Record::REPL_ONLINE:
                wrapped_handler_->on_repl_online();
                break;

            case dumper::Record::REPL_LIFENUM:
                wrapped_handler_->on_repl_lifenum(record->repl_lifenum());
                break;

            case dumper::Record::REPL_CLEARDELETED:
            {
                cg_data_cleardeleted_t msg;
                const dumper::ClearDeleted& clear_deleted = record->repl_cleardeleted();
                msg.table_idx = clear_deleted.table_idx();
                msg.table_rev = clear_deleted.table_rev();
                wrapped_handler_->on_repl_creardeleted(&msg);
                break;
            }

            case dumper::Record::REPL_REPLSTATE:
                wrapped_handler_->on_repl_replstate(record->repl_replstate());
                break;

            case dumper::Record::MQ_REPLY:
            {
                cg_msg_data_t msg;
                const dumper::MsgData& msg_data = record->mq_reply();
                msg.type = CG_MSG_DATA;
                msg.data_size = msg_data.data().size();
                msg.data = (void*)msg_data.data().c_str();
                msg.msg_index = msg_data.msg_index();
                msg.msg_id = msg_data.msg_id();
                msg.msg_name = msg_data.msg_name().c_str();
                msg.user_id = msg_data.user_id();
                msg.addr = msg_data.addr().c_str();
                msg.ref_msg = nullptr;
                wrapped_handler_->on_mq_reply(&msg);
                break;
            }

            case dumper::Record::MQ_TIMEOUT:
            {
                cg_msg_data_t msg;
                const dumper::MsgData& msg_data = record->mq_timeout();
                msg.type = CG_MSG_DATA;
                msg.data_size = msg_data.data().size();
                msg.data = (void*)msg_data.data().c_str();
                msg.msg_index = msg_data.msg_index();
                msg.msg_id = msg_data.msg_id();
                msg.msg_name = msg_data.msg_name().c_str();
                msg.user_id = msg_data.user_id();
                msg.addr = msg_data.addr().c_str();
                msg.ref_msg = nullptr;
                wrapped_handler_->on_mq_timeout(&msg);
                break;
            }

            case dumper::Record::UNKNOWN_MSG_TYPE:
            {
                cg_msg_t msg;
                const dumper::GeneralMsg& gen_msg = record->unknown_msg();
                msg.type = gen_msg.type();
                msg.data_size = gen_msg.data().size();
                msg.data = (void*)gen_msg.data().c_str();
                wrapped_handler_->on_unknown_msg_type(&msg);
                break;
            }

            case dumper::Record::DUMPER_OPEN:
            case dumper::Record::DUMPER_CLOSE:
            default:
                break;
        }

        delete record;
        prev_timestamp = new_timestamp;
        record = reinterpret_cast<dumper::Record*>(read_next(&pos));
    }

    state_ = State::DUMPING;
}


//===============================================================================
void Dumper::on_listener_open()
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::LISTENER_OPEN);
    record.set_timestamp(get_time());

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_listener_open();
}


//===============================================================================
void Dumper::on_listener_close()
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::LISTENER_CLOSE);
    record.set_timestamp(get_time());

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_listener_close();
}

//===============================================================================
void Dumper::on_transaction_begin()
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::TRANSACTION_BEGIN);
    record.set_timestamp(get_time());

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_transaction_begin();
}

//===============================================================================
void Dumper::on_transaction_commit()
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::TRANSACTION_COMMIT);
    record.set_timestamp(get_time());

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_transaction_commit();
}

//===============================================================================
void Dumper::on_stream_data(cg_msg_streamdata_t* msg)
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::STREAM_DATA);
    record.set_timestamp(get_time());

    dumper::StreamData* stream_data = record.mutable_stream_data();
    stream_data->set_data(msg->data, msg->data_size);
    stream_data->set_msg_index(msg->msg_index);
    stream_data->set_msg_id(msg->msg_id);
    stream_data->set_msg_name(msg->msg_name);
    stream_data->set_rev(msg->rev);
    stream_data->set_num_nulls(msg->num_nulls);
    stream_data->set_nulls(msg->nulls, msg->num_nulls);

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_stream_data(msg);
}

//===============================================================================
void Dumper::on_repl_online()
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::REPL_ONLINE);
    record.set_timestamp(get_time());

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_repl_online();
}

//===============================================================================
void Dumper::on_repl_lifenum(uint32_t lifenum)
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::REPL_LIFENUM);
    record.set_timestamp(get_time());
    record.set_repl_lifenum(lifenum);

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_repl_lifenum(lifenum);
}

//===============================================================================
void Dumper::on_repl_creardeleted(cg_data_cleardeleted_t* msg)
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::REPL_CLEARDELETED);
    record.set_timestamp(get_time());

    dumper::ClearDeleted* cleardeleted = record.mutable_repl_cleardeleted();
    cleardeleted->set_table_idx(msg->table_idx);
    cleardeleted->set_table_rev(msg->table_rev);

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_repl_creardeleted(msg);
}

//===============================================================================
void Dumper::on_repl_replstate(std::string replstate)
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::REPL_REPLSTATE);
    record.set_timestamp(get_time());
    record.set_repl_replstate(replstate);

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_repl_replstate(replstate);
}

//===============================================================================
void Dumper::on_mq_reply(cg_msg_data_t* msg)
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::MQ_REPLY);
    record.set_timestamp(get_time());

    dumper::MsgData* mq_reply = record.mutable_mq_reply();
    mq_reply->set_data(msg->data, msg->data_size);
    mq_reply->set_msg_index(msg->msg_index);
    mq_reply->set_msg_id(msg->msg_id);
    mq_reply->set_msg_name(msg->msg_name);
    mq_reply->set_user_id(msg->user_id);
    mq_reply->set_addr(msg->addr);

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_mq_reply(msg);
}

//===============================================================================
void Dumper::on_mq_timeout(cg_msg_data_t* msg)
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::MQ_TIMEOUT);
    record.set_timestamp(get_time());

    dumper::MsgData* mq_timeout = record.mutable_mq_timeout();
    mq_timeout->set_data(msg->data, msg->data_size);
    mq_timeout->set_msg_index(msg->msg_index);
    mq_timeout->set_msg_id(msg->msg_id);
    mq_timeout->set_msg_name(msg->msg_name);
    mq_timeout->set_user_id(msg->user_id);
    mq_timeout->set_addr(msg->addr);

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_mq_timeout(msg);
}


//===============================================================================
void Dumper::on_unknown_msg_type(cg_msg_t* msg)
{
    if (state_ == State::REPLAYING) { return; }

    dumper::Record record;
    record.set_record_type(dumper::Record::UNKNOWN_MSG_TYPE);
    record.set_timestamp(get_time());

    dumper::GeneralMsg* general_msg = record.mutable_unknown_msg();
    general_msg->set_type(msg->type);
    general_msg->set_data(msg->data, msg->data_size);

    write_next(&record);
    if(wrapped_handler_ == nullptr) return;
    wrapped_handler_->on_unknown_msg_type(msg);
}

//===============================================================================
uint64_t Dumper::get_pos()
{
    return endianness_.ntoh64(*current_position_ptr_);
}

//===============================================================================
void Dumper::set_pos(uint64_t pos)
{
    *current_position_ptr_ = endianness_.hton64(pos);
}


//===============================================================================
uint64_t Dumper::get_time()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}


//===============================================================================
void Dumper::write_next(void* record_ptr)
{
    if(record_ptr == nullptr)
    {
        throw DumperException("Record ptr is NULL");
    }

    dumper::Record* record = reinterpret_cast<dumper::Record*>(record_ptr);
    uint16_t record_size = record->ByteSize();
    uint64_t record_size_size = sizeof(uint16_t);

    uint64_t pos = get_pos();

    if(pos + record_size_size + record_size > file_size_)
    {
        // need to close current file and create new one
        pos = open_file_and_get_pos(current_id_ + 1, file_size_);
    }

    // write record size
    *reinterpret_cast<uint16_t*>(data_+pos) = endianness_.hton16(record_size);
    pos += record_size_size;

    // write record
    record->SerializeToArray(reinterpret_cast<void*>(data_+pos), record_size);
    pos += record_size;

    set_pos(pos);
}

//===============================================================================
void* Dumper::read_next(uint64_t* pos_ptr)
{
    uint16_t record_size = 0;
    uint64_t record_size_size = sizeof(uint16_t);

    if((*pos_ptr) + record_size_size > get_pos())  // for message header
    {
        uint64_t last_available_id = get_file_ids_list().back();
        if(current_id_ + 1 > last_available_id)
        {
            return nullptr; // no more volumes to read
        }
        (*pos_ptr) = open_file_and_get_pos(current_id_ + 1, 0);
    }

    // read record size
    record_size = endianness_.ntoh16(*reinterpret_cast<uint16_t*>(data_ + (*pos_ptr)));
    (*pos_ptr) += record_size_size;

    if((*pos_ptr) + record_size > get_pos())  // for message
    {
        uint64_t last_available_id = get_file_ids_list().back();
        if(current_id_ + 1 > last_available_id)
        {
            return nullptr; // no more volumes to read
        }
        (*pos_ptr) = open_file_and_get_pos(current_id_ + 1, 0);
        // read record size from new file
        record_size = endianness_.ntoh16(*reinterpret_cast<uint16_t*>(data_ + (*pos_ptr)));
        (*pos_ptr) += record_size_size;
    }

    // read record
    dumper::Record* record = new dumper::Record();
    record->ParseFromArray(reinterpret_cast<void*>(data_ + (*pos_ptr)), record_size);
    (*pos_ptr) += record_size;

    return reinterpret_cast<void*>(record);
}


//===============================================================================
std::vector<uint64_t> Dumper::get_file_ids_list()
{
    std::vector<uint64_t> ids_list;

    boost::filesystem::path pth(file_path_);
    std::string file_name = pth.filename().string();
    std::string dir_path = pth.parent_path().string();

    if(boost::filesystem::is_directory(dir_path))
    {
        boost::filesystem::directory_iterator di(dir_path);
        boost::filesystem::directory_iterator di_end;
        for( ; di != di_end; di++)
        {
            boost::filesystem::path p = (*di).path();
            if(p.stem() != file_name || !boost::algorithm::starts_with(p.extension().string(), ".vol-"))
            {
                continue;
            }
            ids_list.push_back(static_cast<uint64_t>(std::stoull(p.extension().string().substr(5))));
        }
    }

    std::sort(ids_list.begin(), ids_list.end());
    return ids_list;
}

//===============================================================================
std::string Dumper::compose_file_path(uint64_t id)
{
    return file_path_ + std::string(".vol-") + std::to_string(id);
}


//===============================================================================
uint64_t Dumper::open_file_and_get_pos(uint64_t id, size_t new_file_size)
{
    // need to close current file and create new one
    if(mapped_file_.is_open())
    {
        mapped_file_.close();
    }

    current_id_ = id;
    mapped_file_params_.path = compose_file_path(current_id_);
    mapped_file_params_.new_file_size = new_file_size;  //this means to create new file
    mapped_file_.open(mapped_file_params_);

    if(!mapped_file_.is_open())
    {
        throw DumperException("Can not open memory mapped file");
    }

    data_ = mapped_file_.data();
    current_position_ptr_ = reinterpret_cast<uint64_t*>(data_+0);

    if(state_ == State::REPLAYING)
    {
        return 0 + sizeof(uint64_t); // replay from beginning
    }

    uint64_t pos = get_pos();
    if(pos == 0)
    {
        set_pos(pos + sizeof(uint64_t));
    }
    return get_pos();
}



}



