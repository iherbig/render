# General

* I knew structs were aligned to 4 byte boundaries by default.
  But it's one thing to *know* something and another thing to **know** something.

* Templates are terrible. They make the already bad error messages of C++ even worse.
  And the compiler doesn't seem to be able to read into the template instantiations
  for generating warnings. I'm almost considering limiting my use of templates to
  the generic dynamic array.

# TGA

* TGA images have an 18 byte header and some optional bits.
* The pixel data in TGA images are RLE (run-length encoding) encoded.

# Questions

* When dealing with converting between coordinate systems, what would be the best way
  to actually handle the types? I've run into the terrors of floating point precision
  while working on this and it seems like maybe I'd just be better served never casting
  between ints and floats except when I explicitly want truncation.

* With RLE, it seems you're unable to get a pixel by position. Instead, you have to
  actually parse each run out and track which range of (x, y) coordinates that run
  represents. I'm honestly not sure if that's the expected way to handle it, or if
  there's something I'm missing.

* Since you generally want your platform layer separate from other things (I'm taking
  that as a given), does your platform layer come with a unique, globally accessible
  set of function pointers for allocation? It's an interesting thought. I might want
  to extract windows.h from every file but main.cpp to see how that would go. Maybe
  once I'm done, I can try to make it a bit more cross-platform. Who knows.

