This is an OPC server written in C (build scripts use [SDCC](http://sdcc.sourceforge.net)). Here you will find the following folders:

**core:** The server core, can be used to build server applications for different platforms. It consists of a single method that will start a blocking server (see [opcs_core.h](core/opcs_core.h)). It needs to be linked with modules that implement [transport.h](core/transport.h) (the transport layer) and [env.h](core/env.h) (the application environment).

**msx:** A server implementation for MSX computers. Two applications will be generated, one for MSX-DOS and one for MSX-BASIC, both using TCP (via TCP/IP UNAPI) as the transport layer. Copy [makefile.example](msx/makefile.example) to `makefile` and tune it as appropriate to get started (`makefile` is ignored in source control).<sup>1</sup>

**lib:** External libraries. All of these are taken from [Konamiman's MSX page](http://www.konamiman.com), sources are available at [Konamiman's MSX project in GitHub](http://www.github.com/konamiman/MSX).

**sandbox:** Everything in this folder will be ignored by source control, use it as your playground.

If you want to develop your own server implementation the best way to get started is to look at how the server for MSX is made.

<sup>1</sup> _Windows user? You will need [Make for Windows](http://www.equation.com/servlet/equation.cmd?fa=make)._