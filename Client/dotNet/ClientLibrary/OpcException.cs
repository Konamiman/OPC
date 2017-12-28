using System;

namespace Konamiman.Opc.ClientLibrary
{
    public class OpcException : Exception
    {
        public OpcException(string messageFromServer) : base($"Error received from server: {messageFromServer}")
        {
            this.MessageFromServer = messageFromServer;
        }

        public string MessageFromServer { get; }
    }
}
