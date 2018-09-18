# Renderer

Renderer following: https://github.com/ssloy/tinyrenderer/wiki

# Purpose

To learn how rendering works.

# Win32?

Yes.

ssloy/tinyrenderer utilizes the TGAImage format distributing a header/src file pair. But I wanted to see it running from a window instead of just writing to a file.

So I'm using the win32 API's to handle all of the window creation and pixel blitting.

# Other

I don't really like templates. In my limited experience, they just make debugging more annoying. STL types are even worse.
