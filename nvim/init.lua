-- ahmadinne's Neovim configuration
-- Dependencies : Neovim, ripgrep, unzip, npm


-- Settings
vim.g.mapleader = " "
vim.o.number = true
vim.o.relativenumber = true
vim.o.signcolumn = "yes"
vim.o.wrap = true
vim.o.tabstop = 2
vim.o.softtabstop = 2
vim.o.shiftwidth = 2
vim.o.swapfile = false
vim.o.autochdir = true
vim.o.undofile = true
vim.schedule(function()
	vim.o.clipboard = "unnamedplus"
end)


-- Keymaps
vim.keymap.set('n', '<leader>so', ':update<CR> :source<CR>')
vim.keymap.set('n', '<leader>lf', vim.lsp.buf.format)
vim.keymap.set('n', '<leader>u', vim.cmd.UndotreeToggle)

vim.keymap.set('n', '<leader>b', '<CMD>bnext<CR>')
vim.keymap.set('n', '<leader>B', '<CMD>bprevious<CR>')
vim.keymap.set('n', '<leader>t', '<CMD>tabnext<CR>')
vim.keymap.set('n', '<leader>T', '<CMD>tabprev<CR>')
vim.keymap.set('n', '<leader>w', '<C-w>w')
vim.keymap.set('n', '<leader>W', '<C-w>W')
vim.keymap.set({ 'n', 'o', 'x' }, '{', '<cmd>keepj normal!{<cr>')
vim.keymap.set({ 'n', 'o', 'x' }, '}', '<cmd>keepj normal!}<cr>')

vim.keymap.set('n', 'x', '"_x', opts)
vim.keymap.set('n', 'X', '"_X', opts)
vim.keymap.set('v', 'p', '"_dP', opts)



-- Abbreviation
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
	cnoreabbrev !w :SudaWrite<CR>
	cnoreabbrev !W :SudaWrite<CR>
]])


-- Plugins
require("lazy-setup")
require("lazy").setup({
	{ "lambdalisue/vim-suda" },
	{
		"ahmadinne/paradise.nvim",
		dependencies = { "RRethy/base16-nvim" },
	},
	{
		"nvim-telescope/telescope.nvim", tag = '0.1.8',
		dependencies = { "nvim-lua/plenary.nvim", "nvim-tree/nvim-web-devicons" },
		config = function()
			local actions = require("telescope.actions")
			require('telescope').setup{
				defaults = {
					sorting_strategy = "ascending",
					file_ignore_patterns = {
						".git/*",
						".cache/*",
						"node-modules/*",
					},
					initial_mode = "normal",
					mappings = {
						n = {
							["q"] = actions.close,
						}
					}
				}
			}
		end
	},
	{
		"stevearc/oil.nvim",
		dependencies = { { "nvim-tree/nvim-web-devicons", opts = {} } },
		lazy = false,
		config = function()
			require("oil").setup({
				default_file_explorer = true,
				keymaps = {
					["<esc>"] = { "actions.close", mode = "n" },
					["q"] = { "actions.close", mode = "n" },
					["l"] = { "actions.select", mode = "n" },
					["h"] = { "actions.parent", mode = "n" },
					["."] = { "actions.toggle_hidden", mode = "n" },
				},
			})
		end
	},
	{
		"nvim-neo-tree/neo-tree.nvim",
		branch = "v3.x",
		dependencies = {
			"nvim-lua/plenary.nvim",
			"MunifTanjim/nui.nvim",
			"nvim-tree/nvim-web-devicons",
		},
		lazy = false,
		opts = {
			sources = { "filesystem", "buffers", "git_status" },
			open_files_do_not_replace_types = { "terminal", "Trouble", "trouble", "qf", "Outline" },
			filesystem = {
				bind_to_cwd = false,
				follow_current_file = { enabled = true },
				use_libuv_file_watcher = true,
			},
			window = {
				mappings = {
					["l"] = "open",
					["h"] = "close_node",
					["<space>"] = "none",
					["Y"] = {
						function(state)
							local node = state.tree:get_node()
							local path = node:get_id()
							vim.fn.setreg("+", path, "c")
						end,
						desc = "Copy path to clipboard",
					},
					["O"] = {
						function(state)
							require("lazy.util").open(state.tree:get_node().path, { system = true })
						end,
						desc = "Open with system application",
					},
					["P"] = { "toggle_preview", config = { use_float = false } },
				},
			}
		}
	},
	{
		"folke/lazydev.nvim",
		ft = 'lua',
		opts = {
			library = {
				-- Load luvit types when the `vim.uv` word is found
				{ path = '${3rd}/luv/library', words = { 'vim%.uv' } },
			},
		},
	},
	{
		'neovim/nvim-lspconfig',
		dependencies = {
			{ 'mason-org/mason.nvim', opts = {} },
			'mason-org/mason-lspconfig.nvim',
			'WhoIsSethDaniel/mason-tool-installer.nvim',
			{ 'j-hui/fidget.nvim', opts = {} },
			'saghen/blink.cmp',
		},
		config = function()
			vim.api.nvim_create_autocmd('LspAttach', {
				group = vim.api.nvim_create_augroup('kickstart-lsp-attach', { clear = true }),
				callback = function(event)
					local map = function(keys, func, desc, mode)
						mode = mode or 'n'
						vim.keymap.set(mode, keys, func, { buffer = event.buf, desc = 'LSP: ' .. desc })
					end
					map('grn', vim.lsp.buf.rename, '[R]e[n]ame')
					map('gra', vim.lsp.buf.code_action, '[G]oto Code [A]ction', { 'n', 'x' })
					map('grr', require('telescope.builtin').lsp_references, '[G]oto [R]eferences')
					map('gri', require('telescope.builtin').lsp_implementations, '[G]oto [I]mplementation')
					map('grd', require('telescope.builtin').lsp_definitions, '[G]oto [D]efinition')
					map('grD', vim.lsp.buf.declaration, '[G]oto [D]eclaration')
					map('gO', require('telescope.builtin').lsp_document_symbols, 'Open Document Symbols')
					map('gW', require('telescope.builtin').lsp_dynamic_workspace_symbols, 'Open Workspace Symbols')
					map('grt', require('telescope.builtin').lsp_type_definitions, '[G]oto [T]ype Definition')
					local function client_supports_method(client, method, bufnr)
						if vim.fn.has 'nvim-0.11' == 1 then
							return client:supports_method(method, bufnr)
						else
							return client.supports_method(method, { bufnr = bufnr })
						end
					end

					-- The following two autocommands are used to highlight references of the
					-- word under your cursor when your cursor rests there for a little while.
					--    See `:help CursorHold` for information about when this is executed
					--
					-- When you move your cursor, the highlights will be cleared (the second autocommand).
					local client = vim.lsp.get_client_by_id(event.data.client_id)
					if client and client_supports_method(client, vim.lsp.protocol.Methods.textDocument_documentHighlight, event.buf) then
						local highlight_augroup = vim.api.nvim_create_augroup('kickstart-lsp-highlight', { clear = false })
						vim.api.nvim_create_autocmd({ 'CursorHold', 'CursorHoldI' }, {
							buffer = event.buf,
							group = highlight_augroup,
							callback = vim.lsp.buf.document_highlight,
						})

						vim.api.nvim_create_autocmd({ 'CursorMoved', 'CursorMovedI' }, {
							buffer = event.buf,
							group = highlight_augroup,
							callback = vim.lsp.buf.clear_references,
						})

						vim.api.nvim_create_autocmd('LspDetach', {
							group = vim.api.nvim_create_augroup('kickstart-lsp-detach', { clear = true }),
							callback = function(event2)
								vim.lsp.buf.clear_references()
								vim.api.nvim_clear_autocmds { group = 'kickstart-lsp-highlight', buffer = event2.buf }
							end,
						})
					end
				end,
			})

			-- Diagnostic Config
			-- See :help vim.diagnostic.Opts
			vim.diagnostic.config {
				severity_sort = true,
				float = { border = 'rounded', source = 'if_many' },
				underline = { severity = vim.diagnostic.severity.ERROR },
				signs = vim.g.have_nerd_font and {
					text = {
						[vim.diagnostic.severity.ERROR] = '󰅚 ',
						[vim.diagnostic.severity.WARN] = '󰀪 ',
						[vim.diagnostic.severity.INFO] = '󰋽 ',
						[vim.diagnostic.severity.HINT] = '󰌶 ',
					},
				} or {},
				virtual_text = {
					source = 'if_many',
					spacing = 2,
					format = function(diagnostic)
						local diagnostic_message = {
							[vim.diagnostic.severity.ERROR] = diagnostic.message,
							[vim.diagnostic.severity.WARN] = diagnostic.message,
							[vim.diagnostic.severity.INFO] = diagnostic.message,
							[vim.diagnostic.severity.HINT] = diagnostic.message,
						}
						return diagnostic_message[diagnostic.severity]
					end,
				},
			}

			local capabilities = require('blink.cmp').get_lsp_capabilities()
			local servers = {
				-- clangd = {},
				-- gopls = {},
				jdtls = {},
				pyright = {},
				rust_analyzer = {},
				ts_ls = {},
				lua_ls = {
					-- cmd = { ... },
					-- filetypes = { ... },
					-- capabilities = {},
					settings = {
						Lua = {
							completion = {
								callSnippet = 'Replace',
							},
							diagnostics = { disable = { 'missing-fields' } },
						},
					},
				},
			}

			local ensure_installed = vim.tbl_keys(servers or {})
			vim.list_extend(ensure_installed, {
				'stylua', -- Used to format Lua code
			})
			require('mason-tool-installer').setup { ensure_installed = ensure_installed }

			require('mason-lspconfig').setup {
				ensure_installed = {}, -- explicitly set to an empty table (Kickstart populates installs via mason-tool-installer)
				automatic_installation = false,
				handlers = {
					function(server_name)
						local server = servers[server_name] or {}
						server.capabilities = vim.tbl_deep_extend('force', {}, capabilities, server.capabilities or {})
						require('lspconfig')[server_name].setup(server)
					end,
				},
			}
		end,
	},
	{
		'saghen/blink.cmp',
		event = 'VimEnter',
		version = '1.*',
		dependencies = {
			{
				'L3MON4D3/LuaSnip',
				version = '2.*',
				build = (function()
					if vim.fn.has 'win32' == 1 or vim.fn.executable 'make' == 0 then
						return
					end
					return 'make install_jsregexp'
				end)(),
				opts = {},
			},
			'folke/lazydev.nvim',
		},
		opts = {
			keymap = {
				preset = 'default',
			},
			appearance = {
				nerd_font_variant = 'mono',
			},
			completion = {
				documentation = { auto_show = false, auto_show_delay_ms = 500 },
			},
			sources = {
				default = { 'lsp', 'path', 'snippets', 'lazydev' },
				providers = {
					lazydev = { module = 'lazydev.integrations.blink', score_offset = 100 },
				},
			},
			snippets = { preset = 'luasnip' },
			fuzzy = { implementation = 'lua' },
			signature = { enabled = true },
		},
	},
	{
		'echasnovski/mini.nvim',
		config = function()
			require('mini.surround').setup()
			require('mini.git').setup()
			require('mini.diff').setup()
			-- require('mini.tabline').setup()
			-- local statusline = require 'mini.statusline'
			-- statusline.setup { use_icons = vim.g.have_nerd_font }
			-- statusline.section_location = function()
			-- 	return '%2l:%-2v'
			-- end
		end,
	},
	{
		'lewis6991/gitsigns.nvim',
		opts = {},
	},
	{
		'nvim-lualine/lualine.nvim',
		event = 'VimEnter',
		dependencies = { 'nvim-tree/nvim-web-devicons' },
		opts = {
			options = {
				component_separators = { left = "|", right = "|" },
				section_separators = { left = nil, right = nil },
				globalstatus = true,
			},
			sections = {
				lualine_a = { "mode" },
				lualine_b = { "branch", "filename" },
				lualine_c = { "diff", "diagnostics" },
				lualine_x = { "location" },
				lualine_y = { "encoding" },
				lualine_z = { "filetype" },
			},
		}
	},
	{
		'nvim-treesitter/nvim-treesitter',
		build = ':TSUpdate',
		main = 'nvim-treesitter.configs', -- Sets main module to use for opts
		opts = {
			ensure_installed = { 'bash', 'c', 'diff', 'html', 'lua', 'luadoc', 'markdown', 'markdown_inline', 'query', 'vim', 'vimdoc' },
			auto_install = true,
			highlight = {
				enable = true,
				additional_vim_regex_highlighting = { 'ruby' },
			},
			indent = { enable = true, disable = { 'ruby' } },
		},
	},
	{
		"iamcco/markdown-preview.nvim",
		cmd = { "MarkdownPreviewToggle", "MarkdownPreview", "MarkdownPreviewStop" },
		build = "cd app && npm install",
		init = function()
			vim.g.mkdp_filetypes = { "markdown" }
		end,
		ft = { "markdown" },
	},
	{
		'MeanderingProgrammer/render-markdown.nvim',
		dependencies = { 'nvim-treesitter/nvim-treesitter', 'nvim-mini/mini.nvim' }, -- if you use the mini.nvim suite
		---@module 'render-markdown'
		---@type render.md.UserConfig
		opts = {
			render_modes = { 'n', 'c', 't' },
		},
	},
	{
		'vyfor/cord.nvim',
		build = ':Cord update',
		event = { 'BufReadPost', 'BufNewFile' },
		config = function()
			require("cord").setup()
		end,
	},
	{
		'stevearc/quicker.nvim',
		event = { 'BufReadPost', 'BufNewFile' },
		config = function()
			require("quicker").setup()
		end,
	},
	{
		'rachartier/tiny-inline-diagnostic.nvim',
		event = "VeryLazy",
		priority = 1000,
		config = function()
			require('tiny-inline-diagnostic').setup({
				preset = "simple",
			})
			vim.diagnostic.config({ virtual_text = false })
		end
	},
})


-- Plugin's Keymap
local builtin = require('telescope.builtin')
vim.keymap.set('n', '<leader><leader>', builtin.buffers, { desc = 'Telescope Buffers' })
vim.keymap.set('n', '<leader>f', builtin.find_files, { desc = 'Telescope find files' })
vim.keymap.set('n', '<leader>h', builtin.help_tags, { desc = 'Telescope help tags' })
vim.keymap.set('n', '<leader>g', builtin.live_grep, { desc = 'Telescope live grep' })
vim.keymap.set('n', '<leader>d', builtin.diagnostics, { desc = 'Telecope diagnostics' })
vim.keymap.set('n', '<leader>o', builtin.oldfiles, { desc = 'Telescope recent files' })
vim.keymap.set('n', '<leader>/', function()
	builtin.live_grep {
		grep_open_files = true,
		prompt_title = 'Live Grep in Open Files'
	}
end, { desc = 'Search in Open Files' })
vim.keymap.set('n', '<leader>mp', "<cmd>MarkdownPreview<cr>", { desc = "Markdown Preview" })
-- vim.keymap.set('n', '<leader>e', "<cmd>Oil --float<cr>", { desc = "Oil explorer" })
vim.keymap.set('n', '<leader>e', "<cmd>Neotree filesystem reveal right focus<cr>", { desc = "Neotree" })
vim.keymap.set('n', '<leader>q', function() require("quicker").toggle() end, { desc = "Toggle QuickFix" })


-- Yank Highlight
vim.api.nvim_create_autocmd('TextYankPost', {
	desc = 'Highlight when yanking (copying) text',
	group = vim.api.nvim_create_augroup('kickstart-highlight-yank', { clear = true }),
	callback = function()
		vim.hl.on_yank()
	end,
})

-- Colorscheme Configurations
vim.o.termguicolors = true
vim.o.background = "dark"
vim.g.moonflyTransparent = true
vim.cmd.colorscheme("paradise")
vim.cmd [[
	highlight Normal guibg=none
	highlight NonText guibg=none
	highlight Normal ctermbg=none
	highlight NonText ctermbg=none
	highlight StatusLine guibg=none
]]
