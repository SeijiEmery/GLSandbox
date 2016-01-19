
define.module('gamepad_menu', ['utils.Class', 'ui', 'ext.gl'], function (Class, ui, _gl) {
	var gl = _gl.withResourceContext(this.module)

	var menuButtons = [ { text: "Settings" }, { text: "Modules" }, { text: "OtherThing" } ]

	var menuTextFont = new TextRenderStyle("Some font", 14, "#12af3f");

	var MenuButton = new Renderable({
		init: function () {
			this.shader = new gl.ProceduralShader("shaders/ui/procedural_menu_btn")
			this.quad   = new gl.GeometryUtils.Quad()

			// instanced, buffered handles
			this.backgroundColorHandle = this.shader.getUniform("background_color")
			this.edgeColorHandle       = this.shader.getUniform("edge_color")
			this.etcParamHandle        = this.shader.getUniform("etc_param")

			this.technique = new ProceduralTechnique(this.shader, this.quad)
		},
		release: function () {
			this.shader.release()
			this.quad.release()
			this.technique.release()
		}
		setDimensions: function (rect) {
			this.quad.setDimensions(rect)
		},
		setStateFocused: function () {
			this.edgeColorHandle.setValue(Color4f('#aaffbb', 0.5))
			this.backgroundColorHandle.setValue(Color4f('#10a0c0', 0.9))

			// Note: this does not actually execute any gl calls directly.
			// Instead of writing directly to a uniform, our js sets a value on some _instanced_ uniform data
			// that's tied to our shader handle.
			// ProceduralShader creates this instance (with associated buffers, etc), and ensures that the shader 
			// in question is loaded. Mutiple instances are expected, which is how multiple buttons w/ different
			// states can be rendered from one shader (but multiple procedural shader instances, correlating to
			// the menubutton instances)
		},
		setStateSelected: function () {

		},
		setStateUnselected: function () {

		},
		render: function () {
			this.techinque.execute()
		}
	})

	var menuBar = new UIPane("menu", {
		init: function (prevState) {
			this.state = {
				selectedButton: prevState.selectedButton || 0,
			}

			var menuLayout = new ui.Layout({
				calcWidth: function () {
					return menuPaddingX * 2 + 
						menuSpacingX * (this.children.length - 1) +
						this.children.forEach(function (child) { return child.getWidth(); });
				},
				calcHeight: function () {
					var maxHeight = 0;
					this.children.forEach(function (child) { maxHeight = Math.max(maxHeight, child.getHeight()); })
					return maxHeight + menuPaddingY * 2;
				}
				doLayout: function () {
					var x = (mainWindow.getWidth() - this.getWidth()) * 0.5 + menuPaddingX;
					var y = mainWindow.getHeight() - menuPaddingY;

					this.children.forEach(function (child) {
						child.setLayout(x, y);
						x += child.getWidth() + menuSpacingX;
					});
				}
			})

			// External resources that will need to be released
			this.resources = [];

			var _this = this;
			this.buttons = menuButtons.map(function(b) {
				var layout = new ui.FixedSizeLayout(b.width, b.height);
				var text   = new ui.createText(b.text, menuTextFont); text.attach(layout);

				var button = new MenuButton(); button.bindTo(layout);

				this.resources.add(text);
				this.resources.add(button);

				menuLayout.addChild(layout);

				return {
					layout: layout,
					text: text,
					button: button
				}
			})
		},
		release: function () {
			this.resources.forEach(function (r) {
				r.release();
			})
		},
		selectNextButton: function (dir) {
			var curSelected  = this.state.selectedButton;
			var nextSelected = (this.state.selectedButton + dir) % this.buttons.length;

			setUnselected(this.buttons[curSelected]);
			setSelected  (this.buttons[nextSelected]);

			this.state.selectedButton = nextSelected;
		}
	})

	var menuGamepadController = new GamepadEventHandler({
		init: function () {
			var states = {
				NoMenu: function () {
					this.whenButtonPressed('MENU', function () {
						this.transitionTo('MenuBar')
					})
				},
				MenuBar: function () {
					this.whenButtonPressed('B', function () {
						this.transitionTo('NoMenu')
					})
					this.whenLeftDirectionalInput(function () {
						menuBar.selectNextButton(-1)
					})
					this.whenRightDirectionalInput(function () {
						menuBar.selectNextButton(+1)
					})
					this.whenButtonPressed('A', function () {
						switch (menuBar.selectedButton) {
							case 0: this.transitionTo('SettingsMenu'); break;
							case 1: this.transitionTo('ModuleMenu'); break;
						}
					})
					this.whenButtonPressed('Y', function () {
						this.transitionTo('SearchMenu')
					})
				},
				SettingsMenu: function () {

				},
				Search: function () {

				}
			}
			var transitions = {
				NoMenu: {
					enter: function () {},
					exit: function () {}
				},
				MenuBar: function () {
					enter: function () {
						menuBar.setActive(true)
					},
					exit: function () {
						menuBar.setActive(false)
					}
				},
				SettingsMenu: function () {

				}
			}

			// Assemble
			var that = this;
			this.states = Object.keys(states).map(function (k) {
				var eventHandlers = [];
				states[k].apply({
					whenButtonPressed: function (btn, fcn) {
						eventHandlers.add(function (evt) {
							if (evt.kind & input.gamepad.ButtonPressEvent && evt.button == input.gamepad.buttons[btn]) {
								fcn.apply(that);
								return true;
							}
							return false;
						})
					},
					whenButtonReleased: function (btn, fcn) {

					},
					whenLeftDirectionalInput: function (fcn) {
						eventHandlers.add(function (evt) {
							if ((evt.kind & input.gamepad.ButtonPressEvent && evt.button == input.gamepad.buttons.DPAD_LEFT) ||
								(evt.kind & input.gamepad.AxialEvent && evt.axis == input.gamepad.axes.RIGHT_AXIS && evt.x < 0)) {
								fcn.apply(that);
								return true;
							}
							return false;
						})
					}
				})
				return {
					handleEvent: function (evt) {
						for (var i = eventHandlers.length; i --> 0;) {
							if (eventHandlers[i](evt)) {
								return true;
							}
						}
						return false;
					}
				}
			})
			Object.keys(transitions).forEach(function(k) {
				this.states[k].enter = transitions[k].enter || function () {}
				this.states[k].exit  = transitions[k].exit  || function () {}
			})

			this.activeState = this.states.NoMenu;
			this.defaultState = this.states.NoMenu;
		},
		handleGamepadEvent: function (evt) {
			return this.activeState.handleEvent(evt);
		},
		transitionTo: function (next) {
			utils.assert(this.states[next]);
			if (this.states[next] == this.activeState)
				return;

			if (this.activeState)
				this.activeState.exit()
			this.activeState = this.states[next] || this.defaultState
			if (this.activeState)
				this.activeState.enter()
		},
		saveState: function () {
			return {
				activeState: Object.keys(this.states).reduce(null, function (r, k) {
					return r || this.states[k] == this.activeState;
				}),
			}
		}
		restoreState: function (s) {
			if (this.activeState)
				this.activeState.exit()
			this.activeState = this.states[s.activeState] || this.defaultState
			this.activeState.enter()
		}
	})

	var items = {
		menuGamepadController: menuGamepadController,
		ui_menuBar: menuBar,
	}

	// Serialization system hooks (serialize to/from js objects)
	this.module.onRestoreState(function (state) {
		Object.keys(items).forEach(function (k) {
			items[k].restoreState(state[k])
		})
	})
	this.module.onSaveState(function () {
		var state = {}
		Object.keys(items).forEach(function (k) {
			state[k] = items[k].saveState()
		})
		return state;
	})
	this.module.onExit(function () {
		this.saveState()
		Object.keys(items).forEach(function (k) {
			items[k].release()
		})
	})
})
