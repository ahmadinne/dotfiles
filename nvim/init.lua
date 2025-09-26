-- ahmadinne's Neovim configuration
-- Dependencies : Neovim (Nightly)


-- Settings
vim.g.mapleader = " "
vim.o.number = true
vim.o.relativenumber = true
vim.o.signcolumn = "yes"
vim.o.wrap = true
vim.o.tabstop = 4
vim.o.softtabstop = 4
vim.o.shiftwidth = 4
vim.o.swapfile = false
vim.o.winborder = "rounded"
vim.o.autochdir = true
vim.schedule(function()
	vim.o.clipboard = "unnamedplus"
end)


-- Keymaps
vim.keymap.set('n', '<leader>so', ':update<CR> :source<CR>')
vim.keymap.set('n', '<leader>lf', vim.lsp.buf.format)

vim.keymap.set('n', '<leader>b', '<CMD>bnext<CR>')
vim.keymap.set('n', '<leader>B', '<CMD>bprevious<CR>')
vim.keymap.set('n', '<leader>t', '<CMD>tabnext<CR>')
vim.keymap.set('n', '<leader>T', '<CMD>tabprev<CR>')
vim.keymap.set('n', '<leader>w', '<C-w>w')
vim.keymap.set('n', '<leader>W', '<C-w>W')
vim.keymap.set({ 'n', 'o', 'x' }, '{', '<cmd>keepj normal!{<cr>')
vim.keymap.set({ 'n', 'o', 'x' }, '}', '<cmd>keepj normal!}<cr>')

vim.keymap.set('n', '<leader>dw', '<cmd>close<cr>')
vim.keymap.set('n', '<leader>ds', '<cmd>close<cr>')
vim.keymap.set('n', '<leader>dow', '<cmd>only<cr>')
vim.keymap.set('n', '<leader>db', '<cmd>enew<bar>bd #<cr>')
vim.keymap.set('n', '<leader>ss', '<cmd>vertical new<cr>')

vim.keymap.set('n', 'x', '"_x', opts)
vim.keymap.set('n', 'X', '"_X', opts)
vim.keymap.set('v', 'p', '"_dP', opts)
vim.keymap.set('n', 'H', '^')
vim.keymap.set('n', 'L', '$')
vim.keymap.set('n', 'M', '%')


-- Plugins
vim.pack.add({
	-- Colorscheme Plugins
	{ src = "https://github.com/rrethy/base16-nvim" }, -- Dependencies for paradise.nvim
	{ src = "https://github.com/ahmadinne/paradise.nvim" },

	-- Others Plugins
	{ src = "https://github.com/saghen/blink.cmp" },
	{ src = "https://github.com/L3MON4D3/LuaSnip" }, -- blink.cmp dependencies
	{ src = "https://github.com/folke/lazydev.nvim" }, -- blink.cmp dependencies
	{ src = "https://github.com/folke/flash.nvim" },
	{ src = "https://github.com/stevearc/oil.nvim" },
	{ src = "https://github.com/echasnovski/mini.icons" },
	{ src = "https://github.com/lambdalisue/vim-suda" },
	{ src = "https://github.com/echasnovski/mini.pick" },
	{ src = "https://github.com/neovim/nvim-lspconfig" },
})


-- Plugins Configurations
vim.lsp.enable({ "lua_ls", "bashls", "ts_ls", "vls" })
require "mini.icons".setup()
require "mini.pick".setup()
require "flash".setup()
require "oil".setup({
	default_file_explorer = true,
	columns = { "icons" },
	delete_to_trash = true,
	keymaps = {
		["<esc><esc>"] = { "actions.close", mode = "n" },
		["qq"] = { "actions.close", mode = "n" },
		["L"] = { "actions.select", mode = "n" },
		["H"] = { "actions.parent", mode = "n" },
		["<leader><cr>"] = { "actions.select", opts = { vertical = true } }
	}
})
require "blink.cmp".setup({
	keymap = {
		preset = "default",
	},
	appearance = {
		nerd_font_variant = "mono",
	},
	completion = {
		documentation = { auto_show = false, auto_show_delay_ms = 500 },
	},
	sources = {
		default = { "lsp", "path", "snippets", "lazydev" },
		providers = {
			lazydev = { module = "lazydev.integrations.blink", score_offset = 100 },
		},
	},
	snippets = { preset = "luasnip" },
	fuzzy = { implementation = "lua" },
	signature = { enabled = true },
})
require "luasnip".setup({
	completion = {
		menu = {
			draw = {
				components = {
					kind_icon = {
						text = function(ctx)
							local icon = ctx.kind_icon
							if ctx.item.source_name == "LSP" then
								local color_item = require("nvim-highlight-colors").format(
									ctx.item.documentation,
									{ kind = ctx.kind }
								)
								if color_item and color_item.abbr ~= "" then
									icon = color_item.abbr
								end
							end
							return icon .. ctx.icon_gap
						end,
						highlight = function(ctx)
							local highlight = "BlinkCmpKind" .. ctx.kind
							if ctx.item.source_name == "LSP" then
								local color_item = require("nvim-highlight-colors").format(
									ctx.item.documentation,
									{ kind = ctx.kind }
								)
								if color_item and color_item.abbr_hl_group then
									highlight = color_item.abbr_hl_group
								end
							end
							return highlight
						end,
					},
				},
			},
		},
	},
})
require "lazydev".setup({
	library = {
		{ path = '${3rd}/luv/library', words = { 'vim%.uv' } },
	}
})


-- Plugins Keymaps
vim.cmd([[
	cnoreabbrev W :w
	cnoreabbrev Q :q
	cnoreabbrev Q! :q!
	cnoreabbrev WQ :wq
	cnoreabbrev wQ :wq
	cnoreabbrev Wq :wq
	cnoreabbrev wf :SudaWrite<CR>
	cnoreabbrev wF :SudaWrite<CR>
	cnoreabbrev Wf :SudaWrite<CR>
	cnoreabbrev w! :SudaWrite<CR>
	cnoreabbrev W! :SudaWrite<CR>
]])
vim.keymap.set('n', '<leader>e', require("oil").toggle_float)
vim.keymap.set('n', '<leader><leader>', ':Pick buffers<CR>')
vim.keymap.set('n', '<leader>f', ':Pick files<CR>')
vim.keymap.set('n', '<leader>h', ':Pick help<CR>')
vim.keymap.set('n', 'f',
	function() require("flash").jump({ search = { forward = true, wrap = false, multi_window = false } }) end)
vim.keymap.set('n', 'F',
	function() require("flash").jump({ search = { forward = false, wrap = false, multi_window = false } }) end)
vim.keymap.set('n', 's', function() require("flash").jump() end)
vim.keymap.set('n', 'S', function() require("flash").treesitter() end)


-- Autocommands
-- vim.api.nvim_create_autocmd('LspAttach', {
-- 	callback = function(ev)
-- 		local client = vim.lsp.get_client_by_id(ev.data.client_id)
-- 		if client:supports_method('textDocument/completion') then
-- 			vim.lsp.completion.enable(true, client.id, ev.buf, { autotrigger = true })
-- 		end
-- 	end,
-- })
-- vim.cmd("set completeopt+=noselect")


-- Colorscheme Configurations
vim.cmd.colorscheme("paradise")
vim.cmd(":hi statusline guibg=NONE")
