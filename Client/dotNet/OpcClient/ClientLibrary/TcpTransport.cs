using System;
using System.Collections.Generic;
using System.Net.Sockets;

namespace Konamiman.Opc.ClientLibrary
{
    public class TcpTransport : ITransport
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

        public int Receive(byte[] buffer, int index, int size)
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

        private void ValidateParameters(byte[] buffer, int index, int size)
        {
            if (buffer == null)
                throw new ArgumentException($"{nameof(buffer)} can't be null");

            if (index < 0)
                throw new ArgumentOutOfRangeException($"{nameof(index)} can't be negative");

            if (size < 0)
                throw new ArgumentOutOfRangeException($"{nameof(size)} can't be negative");

            if (index >= buffer.Length)
                throw new ArgumentOutOfRangeException($"{nameof(index)} can't be greater than the size of {nameof(buffer)}");

            if(index + size > buffer.Length)
                throw new ArgumentOutOfRangeException($"{nameof(index)}+{nameof(size)} can't be greater than the size of {nameof(buffer)}");
        }

        public int Send(byte[] buffer, int index, int size)
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
