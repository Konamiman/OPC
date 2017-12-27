using NUnit.Framework;
using System;

namespace Konamiman.Opc.ClientLibrary.Tests
{
    [TestFixture(Category = "Ping")]
    public class PingTests : TestsBase
    {
        [Test]
        public void Gets_proper_reply_to_tests_without_extra_info()
        {
            CreateSut(0x00, 0x07);

            var response = sut.Ping(0x07);
            AssertResponse(response, 0x07);
        }

        [Test]
        public void Gets_proper_reply_to_tests_with_extra_info()
        {
            CreateSut(0x00, 0x37, 0x11, 0x22, 0x33);
            var response = sut.Ping(0x07);
            AssertResponse(response, 0x37, 0x11, 0x22, 0x33);

            CreateSut(0x00, 0x27, 0x11, 0x22, 0x33);
            response = sut.Ping(0x07);
            AssertResponse(response, 0x27, 0x11, 0x22);
        }

        [Test]
        public void Throws_on_server_error()
        {
            AssertThrowsOpcError(() => sut.Ping(0));
        }

        [Test]
        [TestCase(-1)]
        [TestCase(16)]
        public void Throws_on_invalid_command_parameter(int parameter)
        {
            Assert.Throws<ArgumentOutOfRangeException>(() => sut.Ping(parameter));
        }
    }
}
