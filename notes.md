# General

* Struct alignment is a bit more complicated than "four-byte boundaries by default."
  I'm not exactly sure how to explain it. But they align to the largest field when
  laid out in memory. So if a double is the largest type, they'll align to 8 bytes.

* Templates are terrible. They make the already bad error messages of C++ even worse.
  And the compiler doesn't seem to be able to read into the template instantiations
  for generating warnings. I'm almost considering limiting my use of templates to
  the generic dynamic array.

# TGA

* TGA images have an 18 byte header and some optional bits.
* RLE data is meant to be decompressed.

# Questions

* When dealing with converting between coordinate systems, what would be the best way
  to actually handle the types? I've run into the terrors of floating point precision
  while working on this and it seems like maybe I'd just be better served never casting
  between ints and floats except when I explicitly want truncation.

* Since you generally want your platform layer separate from other things (I'm taking
  that as a given), does your platform layer come with a unique, globally accessible
  set of function pointers for allocation? It's an interesting thought. I might want
  to extract windows.h from every file but main.cpp to see how that would go. Maybe
  once I'm done, I can try to make it a bit more cross-platform. Who knows.

* What coordinate system is the obj file defined in? I'm not sure why some z-values are negative.
