using System;
using System.Net.Sockets;

namespace Konamiman.Opc.ClientLibrary
{
    public class TcpTransport : TransportBase
    {
        readonly TcpClient tcpClient;
        readonly string remoteUrl;
        readonly int remotePort;

        public TcpTransport(string url, int port)
        {
            this.tcpClient = new TcpClient();
            this.remoteUrl = url;
            this.remotePort = port;
        }

        public void Connect()
        {
            try
            {
                tcpClient.Connect(remoteUrl, remotePort);
            }
            catch(Exception ex)
            {
                throw new TransportException(
                    $"Could not connect to {remoteUrl}:{remotePort} - {ex.Message}",
                    ex);
            }
        }

        public override int Receive(byte[] buffer, int index, int size)
        {
            ValidateParameters(buffer, index, size);

            if (size == 0) return 0;
            try
            {
                var remaining = size;
                while (remaining > 0)
                {
                    var received = tcpClient.Client.Receive(buffer, index, remaining, SocketFlags.None, out SocketError error); // s.Read(data, retrieved, remaining);
                    if (received == 0) break;
                    index += received;
                    remaining -= received;
                }
                return size - remaining;
            }
            catch(Exception ex)
            {
                throw new TransportException(
                    $"Error when receiving from {remoteUrl}:{remotePort} - {ex.Message}",
                    ex);
            }
        }

        public override int Send(byte[] buffer, int index, int size)
        {
            ValidateParameters(buffer, index, size);

            if (size == 0) return 0;
            try
            {
                var remaining = size;
                while (remaining > 0)
                {
                    var sent = tcpClient.Client.Send(buffer, index, remaining, SocketFlags.None, out SocketError error); // s.Read(data, retrieved, remaining);
                    if (sent == 0) break;
                    index += sent;
                    remaining -= sent;
                }
                return size - remaining;
            }
            catch (Exception ex)
            {
                throw new TransportException(
                    $"Error when sending to {remoteUrl}:{remotePort} - {ex.Message}",
                    ex);
            }
        }
    }
}
