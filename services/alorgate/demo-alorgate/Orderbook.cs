using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Diagnostics;
using System.Threading.Tasks;

using Atentis.Connection;

using RestSharp;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;


namespace alorgate
{
    public class Orderbook
    {
        public class Data
        {
            public double[] prices;
            public double[] volumes;
        }

        Slot slot_ = null;
        Securities securities_ = null;
        public ActiveSecuritiesEntry security { get; set; }
        int max_depth_ = 0;
        RestClient rest_client_ = null;
        Data orderbook_rows_ = new Data();
        int row_index_ = 0;
        
        int total_number_received_ = 0;
        DateTime last_print = DateTime.Now;
        DateTime begin_time = DateTime.Now;

        double total_elapsed_time_ms_ = 0.0;

        public Orderbook(Slot slot, Securities securities, ActiveSecuritiesEntry security, int max_depth, RestClient rest_client)
        {
            slot_ = slot;
            securities_ = securities;
            this.security = security;
            max_depth_ = max_depth;
            rest_client_ = rest_client;
            orderbook_rows_.prices = new double[max_depth * 2];
            orderbook_rows_.volumes = new double[max_depth * 2];
            clear_orderbook_rows();
        }

        void clear_orderbook_rows()
        {
            for (int i = 0; i < max_depth_ * 2; ++i)
            {
                orderbook_rows_.prices[i] = 0.0;
                orderbook_rows_.volumes[i] = 0.0;
            }
            row_index_ = 0;
        }


        public void subscribe()
        {
            Table table = slot_.AddOrderbook(security.sec_board, security.sec_code);
            table.Baseless = true;
            table.evhAddRow += this.AddRow;
            table.evhUpdateRow += this.AddRow;
            table.evhClearData += this.ClearData;
            table.evhBeginFrame += this.BeginFrame;
            table.evhEndFrame += this.EndFrame;
        }

        void AddRow(object sender, TableDataEventArgs e)
        {
            DataRow dr = (DataRow)e.DataRow;

            orderbook_rows_.prices[row_index_] = (double)dr["Price"];
            orderbook_rows_.volumes[row_index_] = (int)dr["Quantity"] * (((string)dr["BuySell"]).Trim() == "B" ? +1.0 : -1.0);
            row_index_++;
        }

        void ClearData(object sender, TableDataEventArgs e)
        {
            clear_orderbook_rows();
        }

        void BeginFrame(object sender, TableDataEventArgs e)
        {
            //pass
        }

        void EndFrame(object sender, TableDataEventArgs e)
        {
            if (!securities_.IsTradeAllowed[security.sec_code])
            {
                return;
            }

            Stopwatch stop_watch = new Stopwatch();
            stop_watch.Start();

            // send data to itex-manager
            var request = new RestRequest("itex-demo/securities/" + security.id + "/orderbook", Method.POST);
            string body = JsonConvert.SerializeObject(orderbook_rows_);
            request.AddParameter("text/json", body, ParameterType.RequestBody);
            var response = rest_client_.Execute(request);

            stop_watch.Stop();

            ++total_number_received_;
            total_elapsed_time_ms_ += stop_watch.Elapsed.TotalMilliseconds;

            double time_period_msec = 1000;
            DateTime end_time = DateTime.Now;
            if ((end_time - last_print).TotalMilliseconds > time_period_msec)
            {
                Console.WriteLine("Elapsed time: " + (total_elapsed_time_ms_ / (double)total_number_received_) + " ms  |  Total orderbooks received: " + total_number_received_); //[" + security.sec_board + "] [" + security.sec_code + "]");
                last_print = end_time;
            }
        }
    }
}
