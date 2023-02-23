#!/bin/sh

set -e

breakline() {
    printf "================================================================================\n\n"
}

# Configured in the CI step
install_prefix="appdir/usr"

# The directory we finally pack into our .rpm package
packaging_dir="rpm"

# Get the Ubuntu Release (e.g. 20.04 or 22.04)
ubuntu_release="$(lsb_release -rs)"

# The final path where we'll save the .rpm package
rpm_path="Chatterino-fedora-${ubuntu_release}-x86_64.rpm"

echo "Building .rpm file on '$ubuntu_release'"

if [ ! -f ./bin/chatterino ] || [ ! -x ./bin/chatterino ]; then
    echo "ERROR: No chatterino binary file found. This script must be run in the build folder, and chatterino must be built first."
    exit 1
fi

chatterino_version=$(git describe 2>/dev/null) || true
if [ "$(echo "$chatterino_version" | cut -c1-1)" = 'v' ]; then
    chatterino_version="$(echo "$chatterino_version" | cut -c2-)"
else
    chatterino_version="0.0.0"
fi

# Make sure no old remnants of a previous packaging remains
rm -vrf "$packaging_dir"

mkdir -p "$packaging_dir/SPECS"

echo "Making control file"
cat >> "$packaging_dir/SPECS/chatterino.spec" << EOF
Name: chatterino
Version: $chatterino_version
Release: $chatterino_version
License: MIT
Summary: RPM package built on $ubuntu_release
URL: https://chatterino.com
Requires: qt5-qtbase-devel qt5-imageformats qt5-qtsvg-devel qt5-linguist libsecret-devel openssl-devel boost-devel

%description
Chatterino RPM package built on $ubuntu_release

%files
../usr/*
EOF
cat "$packaging_dir/SPECS/chatterino.spec"
breakline

echo "Merge install into packaging dir"
mkdir -p "$packaging_dir/BUILDROOT"
find "$install_prefix"
cp -rv "$install_prefix/" "$packaging_dir/BUILDROOT"
find "$packaging_dir"
breakline

echo "Install RPM"
sudo apt install rpm
breakline

echo "Build RPM"
rpmbuild -vv -ba --build-in-place --define "_topdir $(pwd)/rpm" "$packaging_dir/SPECS/chatterino.spec" # full build
breakline

tree "$(pwd)/rpm"

echo "Move RPM up"
mv "$(pwd)/rpm/RPMS/*.rpm" .
breakline

ls -la *.rpm
