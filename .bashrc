# ~/.bashrc


# ---- Environments ----
export PATH=$HOME/.local/bin:$HOME/.config/bin:/usr/bin:/usr/local/bin:$PATH


# ---- Programs ----
eval "$(starship init bash)"


# ---- Options ----
[[ $- != *i* ]] && return
HISTSIZE=1000
SAVEHIST=$HISTSIZE
HISTFILE=~/.history
HISTDUP=erase
bunnyfetch


# ---- Bindings ----
bind '\C-H:backward-kill-word' #Makes control+backspace delete backwards


# ---- Vim Mode ----
# set -o vi
# set keymap vi-command "diw": "bde"
# set keymap vi-command "ciw": "bce"


# ---- Alias ----
alias cat='bat'
alias ls='eza --color=always --git --no-filesize --icons=always --no-time --no-user --no-permissions -a'
alias grep='grep --color=auto'
alias v='$EDITOR'
alias vi='$EDITOR'
alias bf='bunnyfetch'
alias ff="fastfetch"
alias boottime='systemd-analyze'
alias bashrc='$EDITOR $HOME/.config/bashrc'
alias init.lua='$EDITOR $HOME/.config/nvim/init.lua'

alias mem='free --mega'
alias memory='free --mega'
alias disk='echo -e "Filesystem      Size  Used Avail Use% Mounted on"; df -h | grep /boot; df -h | grep /home'


# ---- Function ----
function y(){
	local tmp="$(mktemp -t "yazi-cwd.XXXXXX")" cwd
	yazi "$@" --cwd-file="$tmp"
	if cwd="$(command cat -- "$tmp")" && [ -n "$cwd" ] && [ "$cwd" != "$PWD" ]; then
		builtin cd -- "$cwd"
	fi
	rm -f -- "$tmp"
}
alias yazi=y
