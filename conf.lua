
--[[
App Launch API:
	screen: { width: number, height: number }
	path: { concat: function (string, string) -> string }
]]

function min (a, b)
	return a <= b and a or b
end
function max (a, b)
	return a >= b and a or b
end

console.log("Hello world!")
console.log(console)
console.log(console.log)
console.log(screen)
console.log(screen.width)
console.log(screen.height)
console.log(path)
console.log(path.concat)

window = {}
window.height = screen.height - 40
window.width  = min(window.height * screen.width / screen.height, screen.width - 40)
window.appname = "GL Sandbox"
window.display_fps = true

modules = {}
modules.at_load = {
	'input-event-logger', 'module-event-logger'
}

resources = {}
resources.project_dir = '~/misc-projects/GLSandbox'
resources.asset_dirs = {
	root = path.concat(resources.project_dir, 'assets/'),
	cached = '~/Library/Application Support/cached_data/assets/'
}
resources.script_dirs = {
	lib_compiled = '~/Library/Application Support/GLSandbox/compiled_scripts/lib/',
	ui_compiled  = '~/Library/Application Support/GLSandbox/compiled_scripts/ui/',
	modules_compiled = '~/Library/Application Support/GLSandbox/compiled_scripts/modules/',

	lib_src = path.concat(resources.project_dir, 'script/lib/'),
	ui_src  = path.concat(resources.project_dir, 'script/ui/'),
	module_src = path.concat(resources.project_dir, 'script/modules/')
}
resources.log_dir = '~/Library/Application Support/GLSandbox/last_run/logs/'
resources.backup_logs = {
	dir = '~/Library/Application Support/GLSandbox/backup/logs/',
	keep_last_n_logs = 10
}
resources.persistency = {
	hotload_libs = true,
	hotload_modules = true,
	hotload_ui = true,
	hotload_models = true,
	hotload_textures = true,
	hotload_shaders = true,
	hotload_app_config = true,
}
resources.storage = {
	persistent_data_dir = '~/Library/Application Support/GLSandbox/last_run/data/',
	persistent_data_backups = {
		dir = '~/Library/Application Support/GLSandbox/backups/data/',
		keep_n_backups = 10,
	},
	conf_backup = '~/Library/Application Support/GLSandbox/backup/conf/'
}
resources.filetypes = {
	models = {
		obj = { '.obj' },
		obj_mtl = { '.mtl' },
		fbx = { '.fbx' },
	},
	textures = { '.jpg', '.png' },
	shaders = {
		fragment = { '.fs' },
		vertex   = { '.vs' },
		geometry = { '.gs' },
	},
	script = {
		lua = { '.lua' },
		moonscript = { '.moon' },
		moonscript_compiled = { '.lua' },
	},
}





