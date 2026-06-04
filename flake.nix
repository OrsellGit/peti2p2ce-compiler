{
  description = "Nix flake for the PeTI to P2:CE compilers.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        pkgsCross = import nixpkgs {
          inherit system;
          crossSystem = {
            config = "x86_64-w64-mingw32";
            libc = "ucrt";
          };
        };

        requiredPackages = with pkgs; [
          gdb
          valgrind
          cmake
          libgcc
          codespell
          cppcheck
          doxygen
        ];

        requiredLibsWindows = with pkgs; [
            pkgsCross.windows.pthreads
        ];

      in {
        devShells.default = pkgs.mkShell {
          packages = requiredPackages;
        };

        devShells.windows = pkgsCross.mkShell {
          packages = requiredPackages;
          buildInputs = requiredLibsWindows;
          LD_LIBRARY_PATH = "${pkgs.lib.makeLibraryPath requiredLibsWindows}";
          nativeBuildInputs = [
            pkgs.wine64
          ];
          shellHook = ''
            alias lg='lazygit'
            alias cmake-win-setup='cmake -B cmake-build-debug . -D CMAKE_TOOLCHAIN_FILE=crosscompile_x86_64-w64-mingw32.cmake -D CMAKE_BUILD_TYPE=Debug --fresh'
            alias cmake-win-setup-release='cmake -B cmake-build-release . -D CMAKE_TOOLCHAIN_FILE=crosscompile_x86_64-w64-mingw32.cmake -D CMAKE_BUILD_TYPE=Release --fresh'
            alias cmake-win-build='cmake --build cmake-build-debug --config Debug'
            alias cmake-win-build-release='cmake --build cmake-build-release --config Release'
          '';
        };
      }
    );
}
