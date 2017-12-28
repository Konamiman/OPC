using NUnit.Framework;
using System;
using System.Linq;
using System.Text;

namespace Konamiman.Opc.ClientLibrary.Tests
{
    public abstract class TestsBase
    {
        public FakeTransport transport { get; private set; }
        public OpcClient sut { get; private set; }

        protected void CreateSut(params byte[] receiveBuffer)
        {
            transport = new FakeTransport(receiveBuffer);
            sut = new OpcClient(transport);
        }

        protected void CreateSut(string errorMessageToReturn)
        {
            var receiveBuffer = 
                new byte[] { (byte)errorMessageToReturn.Length }
                .Concat(Encoding.ASCII.GetBytes(errorMessageToReturn))
                .ToArray();

            CreateSut(receiveBuffer);
        }

        protected void AssertThrowsOpcError(Action action)
        {
            CreateSut("Error");

            var ex = Assert.Throws<OpcException>(() => action());
            Assert.AreEqual("Error", ex.MessageFromServer);
        }

        protected void AssertResponse(byte[] responseBytes, params byte[] expectedBytes)
        {
            CollectionAssert.AreEqual(expectedBytes, responseBytes);
        }

        protected ushort RandomAddress => (ushort)new Random().Next(0, ushort.MaxValue);

        protected byte RandomPort => (byte)new Random().Next(0, byte.MaxValue);
    }
}
