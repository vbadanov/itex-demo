using System;
using System.Net;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Runtime.InteropServices;
using System.Threading;

using RestSharp;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace alorgate
{
    class MainClass
	{
		public static void Main (string[] args)
		{
            ServicePointManager.UseNagleAlgorithm = false;
            var rest_client = new RestClient("http://127.0.0.1:1502");
            rest_client.Timeout = 1000;

			//Создаем коннектор к серверу «АЛОР-Трейд»
            Console.WriteLine("Creating ALOR-Trade connector...");

            string password = Encoding.Default.GetString(Convert.FromBase64String("<base64-encoded-password-here>"));
            List<AlorTradeConnector> connectors = new List<AlorTradeConnector>
            {
                new AlorTradeConnector(rest_client, "fut7.alor.ru", 7800, "<login-here>", password),
            };

            foreach (var connector in connectors)
            {
                connector.Connect();
            }
            
            Console.WriteLine("Press any key to continue...");
            Console.ReadKey();
		}
	}
}
