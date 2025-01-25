{
  description = "ss - a radically Simple Shell";

  inputs.nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      eachSystem = f: nixpkgs.lib.genAttrs systems f;
    in
    {
      devShells = eachSystem (system: rec {
        ss =
          let
            pkgs = import nixpkgs { inherit system; };
          in
          pkgs.mkShell {
            packages = with pkgs; [
              cmake
              gdb
              gcc
            ];
          };
        default = ss;
      });
      packages = eachSystem (system: rec {
        ss =
          let
            pkgs = import nixpkgs { inherit system; };
          in
          pkgs.stdenv.mkDerivation {
            name = "ss";
            version = "0.0.1";
            installPhase = ''
              mkdir -p $out/bin
              make 
              mv ss $out/bin
            '';
            src = ./.;
            buildInputs = [ ];
          };
        default = ss;
      });
      apps = eachSystem (system: rec {
        ss = {
          type = "app";
          program = "${self.packages.${system}.ss}/bin/ss";
        };
        default = ss;
        debug =
          let
            pkgs = import nixpkgs { inherit system; };
          in
          {
            type = "app";
            program = "${pkgs.writeShellScript "debug-ss" "${pkgs.gdb}/bin/gdb -ex r ${self.packages.${system}.ss}/bin/ss"}";
          };
      });
    };
}
