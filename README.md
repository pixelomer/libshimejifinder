# libshimejifinder

Library for finding and extracting shimeji from archives

(**Disclaimer:** libshimejifinder is currently not accepting contributions. Any pull requests will be closed without review.)

## Usage

```cpp
auto ar = shimejifinder::analyze("/path/to/archive.zip");
ar->extract("/path/to/mascots/");
std::set<std::string> changedMascots = ar->shimejis();

// Shimeji will be written in the following format:
//
// | mascots/
// |-. Shimeji_1.mascot/
// | |-- behaviors.xml
// | |-- actions.xml
// | |-. img/
// | | |-- shime1.png
// | | |-- shime2.png
// | | '-- [...]
// | '-. sound/
// |   |-- sound1.wav
// |   |-- sound2.wav
// |   '-- [...]
// '-. Shimeji_2.mascot/
//   |-- [...]
```
