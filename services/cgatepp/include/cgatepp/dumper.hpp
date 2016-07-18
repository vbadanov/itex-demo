#ifndef CGATEPP_DUMPER_HPP_
#define CGATEPP_DUMPER_HPP_

#include <string>
#include <vector>
#include <aux/endianness.hpp>
#include <cgatepp/listener.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

namespace cgatepp
{

class Dumper : public Listener::IHandler
{
    public:
        Dumper(std::string file_path, size_t file_size_, Listener::IHandler* wrapped_handler);
        Dumper(const Dumper&) = delete;
        Dumper(Dumper&& dumper) = delete;
        ~Dumper();

        void replay_recorded_stream(bool wait_by_timestamps);

        void on_listener_open();
        void on_listener_close();
        void on_transaction_begin();
        void on_transaction_commit();
        void on_stream_data(cg_msg_streamdata_t* msg);
        void on_repl_online();
        void on_repl_lifenum(uint32_t lifenum);
        void on_repl_creardeleted(cg_data_cleardeleted_t* msg);
        void on_repl_replstate(std::string replstate);
        void on_mq_reply(cg_msg_data_t* msg);
        void on_mq_timeout(cg_msg_data_t* msg);
        void on_unknown_msg_type(cg_msg_t* msg);

    protected:
        enum class State
        {
            DUMPING,
            REPLAYING
        };

        uint64_t get_pos();
        void set_pos(uint64_t pos);

        uint64_t get_time();

        void write_next(void* record_ptr); 		// void pointer to avoid including dumper.pd.h, actually void* = dumper::Record*
        void* read_next(uint64_t* pos_ptr);     // void pointer to avoid including dumper.pd.h, actually void* = dumper::Record*

        std::vector<uint64_t> get_file_ids_list();
        std::string compose_file_path(uint64_t id);
        uint64_t open_file_and_get_pos(uint64_t id, size_t new_file_size); // if new_file_size > 0 then new file will be created

    private:
        std::string file_path_;
        size_t file_size_;
        Listener::IHandler* wrapped_handler_;
        boost::iostreams::mapped_file mapped_file_;
        boost::iostreams::mapped_file_params mapped_file_params_;
        uint64_t current_id_;
        uint64_t* current_position_ptr_;
        char* data_;
        aux::Endianness endianness_;
        State state_;
};


}
#endif //CGATEPP_CONNECTION_HPP_
