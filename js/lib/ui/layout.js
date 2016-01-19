

define('ui.layout', [ 'utils.class', 'ui.impl' ], function (Class, ui) {
	ui.LayoutDirtyFlags = { 
		WIDTH:  1 << 0,
		HEIGHT: 1 << 1
	}

	ui.Layout = new Class({
		calcWidth: NOT_IMPLEMENTED,
		calcHeight: NOT_IMPLEMENTED,
		doLayout: NOT_IMPLEMENTED,
		getWidth: function () {
			if (this.__dirty & ui.LayoutDirtyFlags.WIDTH) {
				this.__dirty &= ~ui.LayoutDirtyFlags.WIDTH;
				this.__width = this.calcWidth();
			}
			return this.__width;
		}
		getHeight: function () {
			if (this.__dirty & ui.LayoutDirtyFlags.HEIGHT) {
				this.__dirty &= ~ui.LayoutDirtyFlags.HEIGHT;
				this.__width = this.calcWidth();
			}
			return this.__width;
		}
		__baseInit: function () {
			this.__dirty = ~0;
			this.__width = 0;
			this.__height = 0;
			this.__x = 0;
			this.__y = 0;
		}
	})

	return ui.Layout;
})
