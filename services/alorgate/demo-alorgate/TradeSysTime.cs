using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Threading.Tasks;

using Atentis.Connection;


namespace alorgate
{
    public class TradeSysTime
    {
        Slot slot_ = null;

        public TradeSysTime(Slot slot)
        {
            slot_ = slot;
        }

        public void subscribe()
        {
            Table table = slot_.AddTable(new Table("TESYSTIME", "TESYSTIME", "", "", "", ""));
            table.Baseless = true;
            table.evhAddRow += this.AddRow;
            table.evhUpdateRow += this.AddRow;
        }

        void AddRow(object sender, TableDataEventArgs e)
        {
            DataRow dr = e.DataRow;
            Console.WriteLine(((DateTime)dr["Time"]).ToString());
        }

    }
}
