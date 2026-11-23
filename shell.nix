{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell.override { stdenv = pkgs.llvmPackages.libcxxStdenv; } {
  packages = with pkgs; [
    clang
    cmake
  ];

  # With libcxxStdenv, this line is often still helpful
  # to ensure the runtime linker finds the correct libc++.
  LD_LIBRARY_PATH = "${pkgs.stdenv.cc.cc.lib}/lib";
}
