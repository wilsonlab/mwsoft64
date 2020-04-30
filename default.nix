{ stdenv, x11, fontconfig, fontadobe75dpi, fontadobe100dpi, python }:

stdenv.mkDerivation rec {
  name = "mwsoft64";
  src = ./.;
  buildInputs = [ x11 fontadobe75dpi fontadobe100dpi fontconfig python ];

  postPatch = ''
    substituteInPlace definitions.mk --replace "/bin/date" "echo Thu Jan 1 00:00:00 UTC 1970"
  '';

  # TODO: create pkg-config files for shared libs
  installPhase = ''
    mkdir -p $out/bin
    cp ./build/bin/* $out/bin/
  '';
  

  doCheck = true;
  meta = {
    description = "Spike analysis utilities";
    longDescription = ''
      Libraries and tools developed in MIT's Wilson Lab for
      analyzing multichannel electrode data
    '';
    homepage = https://github.com/wilsonlab/mwsoft64;
    license = stdenv.lib.licenses.bsd3;
    platforms = stdenv.lib.platforms.all;
  };



}
