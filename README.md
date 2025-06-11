## Dependencies

This project uses [vcpkg](https://github.com/microsoft/vcpkg) for dependency management.

### Setup

```bash
git clone https://github.com/yourname/GameBoy-Project.git
cd GameBoy-Project

# Clone vcpkg if not already installed
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg integrate install
cd ..

# Install dependencies
vcpkg install
```

If this doesn't work, well i don't know, ask stack overflow or something, it's my first time using vcpkg
