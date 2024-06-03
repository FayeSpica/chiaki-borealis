

sudo apt install flatpak flatpak-builder

flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

flatpak install -y flathub org.kde.Platform//5.14
flatpak install -y flathub org.kde.Sdk//5.14
