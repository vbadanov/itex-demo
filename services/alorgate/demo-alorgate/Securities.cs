using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Threading.Tasks;

using Atentis.Connection;
using RestSharp;


namespace alorgate
{
    public class Securities
    {
        Slot slot_ = null;
        RestClient rest_client_ = null;

        public Dictionary<string, bool> IsTradeAllowed { get; set; }

        public Securities(Slot slot, RestClient rest_client)
        {
            slot_ = slot;
            rest_client_ = rest_client;
            IsTradeAllowed = new Dictionary<string, bool>();
        }

        public void subscribe(Table table = null)
        {
            //Подписываемся на таблицу финансовых инструментов — SECURITIES (загрузка таблицы будет начата после подключения)
            //Таблицу SECURITIES всегда нужно заказывать первой (обязательные поля = "SECID,SECBOARD,SECCODE,DECIMALS")
            if (table == null)
            {
                table = slot_.AddTable(new Table("SECURITIES", "SECURITIES", "", "", ""/*"SecBoard,SecCode,TradingStatus,Decimals,MinStep,StepPrice,BuyDeposit,SellDeposit"*/, ""));
            }
            table.evhAddRow += this.AddRow;
            table.evhUpdateRow += this.UpdateRow;
        }

        void AddRow(object sender, TableDataEventArgs e)
        {
            DataRow dr = e.DataRow;
            if (((string)dr["SecCode"]).StartsWith("Si-9.16 "))
            {
                this.IsTradeAllowed[(string)dr["SecCode"]] = ((string)dr["TradingStatus"]).StartsWith("T");
                Orderbook orderbook = new Orderbook(slot_, this, new ActiveSecuritiesEntry { id = 0, alias_id = 0, sec_board = (string)dr["SecBoard"], sec_code = (string)dr["SecCode"] }, 50, rest_client_);
                orderbook.subscribe();
            }
        }

        void UpdateRow(object sender, TableDataEventArgs e)
        {
            DataRow dr = e.DataRow;
            this.IsTradeAllowed[(string)dr["SecCode"]] = ((string)dr["TradingStatus"]).StartsWith("T");
        }

        void EndFrame(object sender, TableDataEventArgs e)
        {
            //pass
        }
    }
}
