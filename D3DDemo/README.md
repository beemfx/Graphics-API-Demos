# D3DDemo

(c) 2003 Beem Media. All rights reserved.

This was written as a learning exercise for Direct3D. It actually started as
Direct3D 8, but was upgraded to Direct3D 9 (and that is the only version that
exists today). It in itself is not very interesting, and it's not even useful
since DirectX 9 is obsolete (and this used the fixed function pipeline on top
of that).

It does feature MD3 file support (the Quake 3 Arena file format) fairly
robustly (but again fixed function pipeline so no shader support). Which is
probably the most interesting thing about it. It really exists only for novelty
at this point, and doesn't even represent my capabilities as a game programmer.
Being that it is over ~~13~~ 22 years old.

I played around with this a bit in 2026 because I wanted to get the MD3 files to load from actual PK3 files. I also refactored a bunch of stuff and have separated the MD3 file data away from the DirectX implementation.
