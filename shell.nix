{ pkgs ? import <nixpkgs> {} }:

pkgs.callPackages ./default.nix {}
