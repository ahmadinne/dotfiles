#!/usr/bin/env bash

packages=(
	"brightnessctl"
	"cclip"
	"git"
	"gnome-keyring"
	"greetd"
	"grim"
	"gvfs"
	"hyprland"
	"hyprpicker"
	"imagemagick"
	"imv"
	"less"
	"libnotify"
	"mako"
	"man-db"
	"mate-polkit"
	"neovim"
	"networkmanager"
	"npm"
	"pcmanfm"
	"rofi"
	"swaybg"
	"tela-circle-icon-theme-standard"
	"tree"
	"ttf-nerd-fonts-symbols"
	"waybar"
	"wl-clipboard"
	"xarchiver"
	"xdg-desktop-portal-hyprland"
	"xdg-user-dirs"
	"yazi"
)

# Aur helper
aur=yay
if [ ! $(pacman -Qqe ${aur}-bin 2>/dev/null) ]; then
	[ ! $(pacman -Qqe base-devel 2>/dev/null) ] && sudo pacman -S base-devel
	git clone https://aur.archlinux.org/${aur}-bin
	(cd ${aur}-bin && makepkg -si) && rm -rf ${aur}-bin
	sudo pacman -Rnsc ${aur}-bin-debug
fi

[[ $(pacman -Qqe ${aur}-bin 2>/dev/null) ]] || exit 1

for pkg in "${packages[@]}"; do
	if [ ! $(pacman -Qqe $pkg 2>/dev/null) ]; then
		$aur -S --noconfirm $pkg &&\
			echo "$pkg installed" ||\
			echo "$pkg failed to install"
	else
		echo "$pkg already installed"
	fi
done

# Downloading Alacritty (terminal) with image protocol patch. -- BINARY --
g_release="https://github.com/ayosec/alacritty/releases/latest/download"
g_list=(
	"alacritty-linux-x86_64.gz"
	"Alacritty.desktop"
	"Alacritty.svg"
	"alacritty.bash"
	"alacritty.1.gz"
	"alacritty.5.gz"
	"alacritty-msg.1.gz"
	"alacritty-bindings.5.gz"
	"alacritty-escapes.7.gz"
)

for list in "${g_list[@]}"; do
	name=${list%.gz}
	[ ! -f "${PWD}/${name}" ] && curl -sL -o "$list" "${g_release}/${list}" && echo "curl'd $list"
	if [[ "$list" == *".gz" ]]; then
		gunzip "$list"
	fi
done

sudo desktop-file-install -m 644 --dir "/usr/share/applications/" "Alacritty.desktop"
sudo install -D -m755 "alacritty-linux-x86_64" "/usr/bin/alacritty"
sudo install -D -m644 "alacritty.bash" "/usr/share/bash-completion/completions/alacritty"
sudo install -D -m644 "Alacritty.svg" "/usr/share/pixmaps/Alacritty.svg"

sudo install -D -m644 "alacritty.1" "/usr/share/man/man1/alacritty.1"
sudo install -D -m644 "alacritty-msg.1" "/usr/share/man/man1/alacritty-msg.1"
sudo install -D -m644 "alacritty.5" "/usr/share/man/man5/alacritty.5"
sudo install -D -m644 "alacritty-bindings.5" "/usr/share/man/man5/alacritty-bindings.5"
sudo install -D -m644 "alacritty-escapes.7" "/usr/share/man/man7/alacritty-escapes.7"
rm [A,a]lacritty*
