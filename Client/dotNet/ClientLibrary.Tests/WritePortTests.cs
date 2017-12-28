using Konamiman.Z80dotNet;
using NUnit.Framework;
using System;
using System.Linq;

namespace Konamiman.Opc.ClientLibrary.Tests
{
    [TestFixture(Category = "Write Port")]
    public class WritePortTests : TestsBase
    {
        [Test]
        [TestCase(7, true)]
        [TestCase(7, false)]
        [TestCase(0x1234, true)]
        [TestCase(0x1234, false)]
        public void Sends_proper_command_bytes(int size, bool autoIncrement)
        {
            byte port = RandomPort;

            var bytesToWrite = Enumerable.Range(1, size).Select(x => (byte)x).ToArray();

            var expectedToSend = (size <= 7 ?
                new byte[] {
                    (byte)(0x50 | size | (autoIncrement ? (1 << 3) : 0)),
                    port }
                :
                new byte[] {
                    (byte)(0x50 | (autoIncrement ? (1 << 3) : 0)),
                    port,
                    size.ToUShort().GetLowByte(), size.ToUShort().GetHighByte() })

                .Concat(bytesToWrite).ToArray();
            
            CreateSut(0);

            sut.WriteToPort(port, bytesToWrite, autoIncrement);

            CollectionAssert.AreEqual(expectedToSend, transport.SentBytes);
        }

        [Test]
        public void Throws_on_server_error()
        {
            AssertThrowsOpcError(() => sut.WriteToPort(RandomPort, new byte[] {1}, false));
        }

        [Test]
        public void Throws_on_null_buffer()
        {
            CreateSut(0);
            Assert.Throws<ArgumentNullException>(() => sut.WriteToPort(RandomPort, null, false));

            CreateSut(0);
            Assert.Throws<ArgumentNullException>(() => sut.WriteToPort(RandomPort, null, 0, 1, false));
        }

        [Test]
        [TestCase(-1, 1)]
        [TestCase(1, -1)]
        [TestCase(11, 1)]
        [TestCase(9, 2)]
        public void Throws_on_bad_size_or_index(int index, int size)
        {
            CreateSut(0);
            var buffer = new byte[10];
            Assert.Throws<ArgumentOutOfRangeException>(() => sut.WriteToPort(RandomPort, buffer, index, size, false));
        }
    }
}
