using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Runtime.InteropServices;
using System.Threading;

using Atentis.Connection;

using RestSharp;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace alorgate
{
    class AlorTradeConnector
    {
        const int ORDERBOOK_MAX_DEPTH = 50;

        Securities securities_ = null;
        TradeSysTime trade_sys_time_ = null;
        List<Orderbook> orderbooks_ = null;
        Slot slot_ = null;
        RestClient rest_client_ = null;
        List<ActiveSecuritiesEntry> active_securities_ = null;
        string server_ = null;
        int port_ = 0;
        string login_ = null;
        string password_ = null;

        public AlorTradeConnector(RestClient rest_client, string server, int port, string login, string password)
        {
            rest_client_ = rest_client;
            server_ = server;
            port_ = port;
            login_ = login;
            password_ = password;
        }

        /// <summary>
        /// Метод для подключения к торговой системе «АЛОР-Трейд»
        /// </summary>
        public void Connect()
        {
            slot_ = new Slot();

            //создаем внутренний объект, управляющий сетевым соединением (RequestSocket)
            slot_.rqs = new RequestSocket(slot_);
            slot_.rqs.Init();
            
            securities_ = new Securities(slot_, rest_client_);
            securities_.subscribe();

            trade_sys_time_ = new TradeSysTime(slot_);
            trade_sys_time_.subscribe();

            Table table;

            //Подписываемя на таблицу заявок — ORDERS
            //Устанавливаем безбазовый режим Baseless = true, который означает, 
            //что таблица ORDERS не будет сохраняться во внутреннем DataSet-е АЛОР-SDK.
            //Подписываемся на события добавления и обновления строк
            table = slot_.AddTable(new Table("ORDERS", "ORDERS", "", "", "", ""));
            table.Baseless = true;
            table.evhAddRow += OrderTableAddRowHandler;
            table.evhUpdateRow += OrderTableUpdateRowHandler;            
           
            //ключ путой (если ключ используются, сюда должен быть установлен путь к ключу)
            slot_.PublicKeyFile = "";
            
            
            //Настраиваем параметры подключения к серверу
            slot_.Server = server_;
            slot_.Port = port_;
            slot_.Login = login_;
            slot_.Password = password_;
  
            slot_.MaxOrderbookSize = ORDERBOOK_MAX_DEPTH * 2;
            slot_.RefreshPeriod = 0;

            //Подключаем все необходимые обработчики событий
            setSlotEventHandlers(slot_);
            setRSocketEventHandlers(slot_);

            slot_.Start();            
        }

        void rqs_UpdateAllTableViews(object sender, TableListEventArgs e)
        {
            foreach (Table t in e.List)
            {
                Console.WriteLine("[" + t.Name + "] RowCount: " + t.GetData().Rows.Count);
                DataTable dt = t.GetData();
                foreach (DataRow dr in dt.Rows)
                {
                    
                    Console.Write("[" + slot_.Server + "] [" + dt.TableName + "] row: ");
                    foreach (DataColumn col in dt.Columns)
                    {
                        Console.Write(col.ColumnName + ": " + dr[col.ColumnName] + " | ");
                    }
                    Console.WriteLine();
                    
                }
                t.ClearData();
            }
        }
        

        void OrderTableAddRowHandler(object sender, TableDataEventArgs e)
        {
            //Этот обработчик вызывается при добавлении новой строки таблицы ORDERS
            //Запускаем асинхронный обработчик события в другом потоке
            //Это делается для того, чтобы вернуть управление потоку выполнения АЛОР-SDK
            ThreadPool.QueueUserWorkItem(OrderTableAddRowAsync, e.DataRow);
        }

        void OrderTableAddRowAsync(Object obj)
        {
            //Асинхронный обработчик события
            //DataRow dr = (DataRow)obj;
            //Console.WriteLine(dr.ToString());
        }

        void OrderTableUpdateRowHandler(object sender, TableDataEventArgs e)
        {
            //Эта функция вызывается при обновлени строки таблицы ORDERS
            //Запускаем асинхронный обработчик события в другом потоке
            //Это делается для того, чтобы вернуть управление потоку выполнения АЛОР-SDK (Atentis)
            ThreadPool.QueueUserWorkItem(OrderTableUpdateRowAsync, e.DataRow);
        }

        void OrderTableUpdateRowAsync(Object obj)
        {
            //Асинхронный обработчик события
            //DataRow dr = (DataRow)obj;
            //Console.WriteLine(dr.ToString());
        }

        void boards_row_handler(object sender, TableDataEventArgs e)
        {
            DataRow dr = (DataRow)e.DataRow;
            Console.Write("Boards-add/upd: ");
            foreach (DataColumn col in dr.Table.Columns)
            {
                Console.Write(col.ColumnName + ": " + dr[col.ColumnName] + " | ");
            }
            Console.WriteLine();
        }

        private double total_direction_ = 0;
        void AllTradesTableAddRowHandler(object sender, TableDataEventArgs e)
        {
            ThreadPool.QueueUserWorkItem(AllTradesTableAddRowHandler_async, e.DataRow);
        }

        void AllTradesTableAddRowHandler_async(Object obj)
        {
            DataRow dr = (DataRow)obj;
			total_direction_ += ((double)dr["Price"]) * (((byte)(dr["RowState"])).Equals(0x40) ? (int)dr["Quantity"] : -(int)dr["Quantity"]);
			Console.WriteLine("Trade-Add: " + dr["TradeNo"] + " | " + dr["TradeTime"].ToString() + " | " + dr["SecBoard"] + " " + dr["SecCode"] + " " + dr["Price"] + " " + (((byte)(dr["RowState"])).Equals(0x40) ? (int)dr["Quantity"] : -(int)dr["Quantity"]) + "    (" + ((double)total_direction_/(double)dr["Price"]) + ")");
        }

        /// <summary>
        /// Добавление обработчиков событий слота
        /// </summary>
        /// <param name="slot">слот</param>
        void setSlotEventHandlers(Slot slot)
        {
            slot.evhSlotStateChanged += new SlotEventHandler(slot_evhSlotStateChanged);
        }

        /// <summary>
        /// Удаление обработчиков событий слота
        /// </summary>
        /// <param name="slot">слот</param>
        void removeSlotEventHandlers(Slot slot)
        {
            slot.evhSlotStateChanged -= slot_evhSlotStateChanged;

        }

        /// <summary>
        /// Добавление обработчиков событий внутренненго объекта RequestSocket
        /// </summary>
        /// <param name="slot">слот</param>
        void setRSocketEventHandlers(Slot slot)
        {
            slot.rqs.evhLoggedIn += rqs_evhServiceLoggedIn;
            slot.rqs.evhNewSession += rqs_evhNewSession;
            slot.rqs.evhNeedNewPassword += rqs_evhNeedNewPassword;
            slot.rqs.evhLogLine += rqs_evhLogLine;
        }

        /// <summary>
        /// Удаление обработчиков событий внутренненго объекта RequestSocket
        /// </summary>
        /// <param name="slot">слот</param>
        void removeRSocketEventHandlers(Slot slot)
        {
            slot.rqs.evhLoggedIn -= rqs_evhServiceLoggedIn;
            slot.rqs.evhNewSession -= rqs_evhNewSession;
            slot.rqs.evhNeedNewPassword -= rqs_evhNeedNewPassword;
            slot.rqs.evhLogLine -= rqs_evhLogLine;
        }


        void rqs_evhLogLine(object sender, TableEventArgs e)
        {
            //Внутренний лог АЛОР-SDK (Atentis)
            //Очень удобен для отладки (в случае возникновения проблем)
            Console.WriteLine("[{0:hh:mm:ss}] EVENT: Slot state changed, state='{1}'", DateTime.Now, e.Message);
        }


        void rqs_evhServiceLoggedIn(object sender, TableEventArgs e)
        {
            //Успешная авторизация на сервере (успешный вход)
            Console.WriteLine("[{0:hh:mm:ss}] EVENT: LoggedIn", DateTime.Now);

        }

        void slot_evhSlotStateChanged(object sender, SlotEventArgs e)
        {
            //Изменился статус подключения
            Console.WriteLine("[{0:hh:mm:ss}] EVENT: Slot state changed, state='{1}'", DateTime.Now, e.State.ToString());
            if (e.State == SlotState.Denied)
            {
                //Доступ запрещен (сервер, доступен, но введен неверный логин или пароль)                
            }
            else if (e.State == SlotState.Failed)
            {
                //В случае прихода Failed ничего не делаем, Atentis сам будет пробовать восстановить сессию
            }
            else if (e.State == SlotState.Disconnected)
            {

            }

        }

        void rqs_evhNewSession(object sender, TableEventArgs e)
        {
            //Требуется создание новой сессии. Работа с текущей сессией далее невозможна
            Console.WriteLine("[{0:hh:mm:ss}] EVENT: New session required", DateTime.Now);
            System.Threading.ThreadPool.QueueUserWorkItem(reconnectSlotAsync, e);

        }

        /// <summary>
        /// Переподключение к серверу «АЛОР-Трейд»
        /// </summary>
        /// <param name="obj">null</param>
        void reconnectSlotAsync(Object obj)
        {
            //Создаем новое соединение
            //Отписываемся от всех событий (обязательно)
            removeSlotEventHandlers(slot_);
            removeRSocketEventHandlers(slot_);

            slot_.Disconnect();
            slot_.Dispose();
            System.Threading.Thread.Sleep(1000);

            Connect();
        }

        void rqs_evhNeedNewPassword(object sender, TableEventArgs e)
        {
            //Сервер "АЛОР-Трейд" требует ввести новый пароль перед продолжением работы
            //Это требование необходимо проигнорировать, в результате чего придет событие Denied и будет установлен текущий пароль
            //При переподключении связь будет восстановленна
            Console.WriteLine("[{0:hh:mm:ss}] EVENT: Need new password", DateTime.Now);

        }
    }
}
