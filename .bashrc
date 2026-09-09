# ~/.bashrc
[[ $- != *i* ]] && return
[[ $0 != *bash* ]] && return

# Exports
HISTCONTROL=ignoredups
export EDITOR="nvim"
export VISUAL="$EDITOR"
export PATH="~/.config/bin/:/usr/local/bin/:$PATH"
export LESS="-R~"
export XDG_DATA_HOME="$HOME/.local/share"
export XDG_CACHE_HOME="$HOME/.cache"
export XDG_CONFIG_HOME="$HOME/.config"
export XDG_RUNTIME_DIR=/run/user/$(id -u)
if [ ! -d "$XDG_RUNTIME_DIR" ]; then
	mkdir -p "$XDG_RUNTIME_DIR"
	chmod 0700 "$XDG_RUNTIME_DIR"
fi

# Aliases
alias ls='ls --color=auto'
alias grep='grep --color=auto'
alias boot='systemd-analyze'
alias disk='echo -e "Filesystem      Size  Used Avail Use% Mounted on"; df -h | grep /nvme0n1p*; df -h | grep /sda'
alias foot.ini="nvim ${HOME}/.config/foot/foot.ini"
alias init.lua="nvim ${HOME}/.config/nvim/init.lua"
y() {
	local tmp="$(mktemp -t "yazi-cwd.XXXXXX")" cwd
	command yazi "$@" --cwd-file="$tmp"
	IFS= read -r -d '' cwd < "$tmp"
	[ "$cwd" != "$PWD" ] && [ -d "$cwd" ] && builtin cd -- "$cwd"
	rm -f -- "$tmp"
}; alias yazi=y
vi() {
	command nvim "$@"

	if [ -f /tmp/nvim_last_dir ]; then
		mapfile -t n_dir < /tmp/nvim_last_dir
		cd "$n_dir"
		rm /tmp/nvim_last_dir
	fi
}; alias nvim=vi

# functions
function fcd {
	fff "$@"
	local NEW=$(<"$HOME/.cache/fff/.fff_d")
	[ "$NEW" != "$PWD" ] && builtin cd "$NEW" && echo "cd $NEW" >> "$HISTFILE"
}; alias ll=fcd; alias fff=fcd

function usage() {
	local choice=$1
	echo "$choice data:"
	ps aux | grep "$choice" | awk '{sum=sum+$3}; END {print "cpu usage: " sum "%"}'
	ps aux | grep "$choice" | awk '{sum=sum+$6}; END {print "ram usage: " sum/1024 " MB"}'
}
alias wusage='usage waybar'
alias husage='usage hypr'
alias dusage='usage dwl'
alias qusage='usage qs'

# Prompt
fclear() {
	clear
	history -s clear
}
bind '"\C-l": "\C-a\C-kclear\n\C-y"'
bind -m vi-command -x '"\e": hyprctl -q dispatch sendshortcut Control+Shift, Space,'
bind -m vi-command -x '"\C-l": fclear'
bind -m vi-insert -x '"\C-l": fclear'

dirty() {
	local sc_num=0; local nc_num=0; local un_num=0; local section=""
	mapfile -t unpushed < <(git cherry 2>/dev/null); unpushed="${#unpushed[@]}"
	while IFS= read -r line; do
		[[ "$line" == "Changes to be committed:" ]] && section="staged" && continue
		[[ "$line" == "Changes not staged for commit:" ]] && section="unstaged" && continue
		[[ "$line" == "Untracked files:" ]] && section="untracked" && continue
		[[ -z "$line" ]] && section="" && continue
		[[ "$line" == *"("*")"* ]] && continue
		[[ "$section" == "untracked" ]] && (( un_num++ ))
		[[ "$section" == "unstaged" ]] && (( nc_num++ ))
		[[ "$section" == "staged" ]] && (( sc_num++ ))
	done < <(git status 2>/dev/null)
    [[ "$un_num" -gt 0 ]] && printf "\033[34m-add(new) "
    [[ "$nc_num" -gt 0 ]] && printf "\033[35m-add(changes) "
	[[ "$sc_num" -gt 0 ]] && printf "\033[32m-commit "
	[[ "$unpushed" -gt 0 ]] && printf "\033[36m-push "
}

branch() {
	local is_branch="$(git branch --show-current 2>/dev/null)"
	[[ -n "$is_branch" ]] && printf "\033[33m(${is_branch})"
}

cwd() {
	local is_branch="$(git branch --show-current 2>/dev/null)"
	local current="$PWD"; current="${current/$HOME/\~}"
	local dir_path="${current%/*}"
	local dir_name="/${current##*/}"
	[[ "$dir_name" == "/~" ]] && dir_name=""
	[[ -n "$is_branch" ]] && \
		printf "\033[2;37m${dir_path}\033[0;33m${dir_name}" || \
		printf "\033[2;37m${dir_path}\033[0;37m${dir_name}"
}

get_cmd() {
	prev_cmd="$curr_cmd"
	curr_cmd="$BASH_COMMAND"
}
trap 'get_cmd' DEBUG

newline() {
	if [[ "$prev_cmd" != "prompt" && "$prev_cmd" != "clear" && "$prev_cmd" != "reset" ]]; then
		printf "\n$(cwd)"
	else
		printf "$(cwd)"
	fi
}

function prompt {
	local __nl='$(newline)'
	local __branch='$(branch)'
	local __dirty='$(dirty)'
    local __none='\033[00m'
	export PS1="$__nl $__branch $__dirty$__none\n "
}

prompt && bf
