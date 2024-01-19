{
  description = "Score-P Plugin using libsensors";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-23.11"; 
    nix-scorep = {
      url = "github:s9105947/nix-scorep";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { self, nixpkgs, nix-scorep }:
    {
      overlays.default = selfpkgs: pkgs: let
        myCallPackage = pkgs.lib.customisation.callPackageWith (pkgs // (nix-scorep.packages.${pkgs.system}));
      in {
        scorep_plugin_libsensors = myCallPackage (
          { stdenv
          , cmake
          , scorep
          , lm_sensors
          }: stdenv.mkDerivation {
            name = "scorep_plugin_libsensors";
            # note: submodules are required
            # to build from current directory use:
            # nix build '.?submodules=1'
            src = ./.;

            nativeBuildInputs = [
              cmake
            ];

            cmakeFlags = [
              "-DSENSORS_DIR=${lm_sensors}"
            ];

            buildInputs = [
              lm_sensors
              scorep
            ];
          }) {};
      };

      packages.x86_64-linux =
        let
          pkgs = import nixpkgs { system = "x86_64-linux"; };
          selfpkgs = self.packages.x86_64-linux;
        in self.overlays.default selfpkgs pkgs;

      defaultPackage.x86_64-linux = self.packages.x86_64-linux.scorep_plugin_libsensors;
    };
}
