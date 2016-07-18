using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Security;

namespace Microsoft.VisualBasic.Devices
{
    public class ComputerInfo
    {
        [SecuritySafeCritical]
        public ComputerInfo()
        {
        }

        [CLSCompliant(false)]
        public ulong AvailablePhysicalMemory
        {
            get
            {
                return 1024 * 1024 * 1024UL;
            }
        }

        [CLSCompliant(false)]
        public ulong TotalPhysicalMemory
        {
            get
            {
                return 4 * 1024 * 1024 * 1024UL;
            }
        }
    }
}
