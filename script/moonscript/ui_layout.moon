
-- test = (thing, do_test) ->
--     do_test(thing) or error("Test failed (" .. thing .. ")")

test = (testcase, do_test) ->
    ok, msg = pcall(-> do_test(thing) or error("test condition failed"))
    ok or error(msg .. "Test failed (" .. testcase .. ")")

class Rect
    new: (x, y, width, height) =>
        @x = x or 0
        @y = y or 0
        @width = width or 0
        @height = height or 0
    topLeft: =>
        return @x, @y
    btmRight: =>
        return @x + @width, @y + @height
    getDimensions: =>
        return @width, @height
    getRect: =>
        return self

test Rect, =>
    test "Rect", =>
        r = Rect(10, 12, 130, 140)
        r.x == 10 and
        r.y == 12 and
        r.width == 130 and
        r.height == 140 and
        r\topLeft! == 10, 12 and
        r\btmRight! == 140, 153 and
        r\getDimensions! == 130, 140

REQUIRE_IMPLEMENTATION = (method_name) ->
    () => 
        error("UNIMPLEMENTED METHOD '" .. method_name .. "'")

test "REQUIRE_IMPLEMENTATION", =>
    not pcall(REQUIRE_IMPLEMENTATION("pcall should fail"))

-- test
-- pcall(REQUIRE_IMPLEMENTATION("pcall should fail")) and
--     print("Failed test: REQUIRE_IMPLEMENTATION")

class Layout
    parent: nil
    rect: Rect!
    __dirtyDimensions: true,
    __dirtyPosition: true

    getDimensions: () =>
        if @__dirtyDimensions:
            @__dirtyDimensions = false
            @rect.width, @rect.height = @calcDimensions()
        return @rect.width, @rect.height
    getRect: () =>
        if @__dirtyDimensions:
            @__dirtyDimensions = false
            @rect.width, @rect.height = @calcDimensions()
        return @rect
    setPosition: (x, y) =>
        @rect.x, @rect.y = x, y
    calcDimensions: REQUIRE_IMPLEMENTATION ("calcDimensions")
    doLayout: REQUIRE_IMPLEMENTATION ("doLayout")

test Layout, =>
    class Stub extends Layout
        @calledCalcDimensions = false
        @calledDoLayout = false
        calcDimensions: () =>
            @calledCalcDimensions = true
            return 13, 42
        doLayout: () =>
            @calledDoLayout = true
    class BadImpl extends Layout
        @foo = 12

    (test "Layout:getDimensions() calls calcDimensions()", =>
        instance = Stub()
        instance\getDimensions() == 13, 42 and
        instance\calledCalcDimensions == true and
        instance\calledDoLayout == false
    ) and (
    test "Layout:getRect() calls calcDimensions()", =>
        instance = Stub()
        instance\getRect().width == 13 and
        instance\getRect().height == 42 and
        instance\calledCalcDimensions == true and
        instance\calledDoLayout == false
    ) and (
    test "Layout:getDimensions() and Layout:getRect() set flags", =>
        a, b = Stub(), Stub()
        a\getDimensions()
        b\getRect()
        a.calledCalcDimensions, b.calledCalcDimensions = false, false
        a\getDimensions()
        b\getDimensions()
        a.calledCalcDimensions == true and
        b.calledCalcDimensions == true
    ) and (
    test "Layout implementations missing calcDimensions and doLayout throw errors", =>
        ok_impl = Stub()
        bad_impl = BadImpl()
        pcall(ok_impl\getDimensions()) and
        pcall(ok_impl\getRect()) and
        not pcall(bad_impl\getDimensions()) and
        not pcall(bad_impl\getRect())
    )





class Hierarchial
    children: {}

    addChild: (child) =>
        table.insert(@children, child)
        child.parent = self

    addAll: (children) =>
        for child in children
            @addChild(child)

class HorizontalLayout extends Layout, Hierarchial
    new: (params) =>
        @padding = params.padding

    calcDimensions: () =>
        width, height = @padding * (@children.length -1), 0
        for child in @children
            w, h = child\getDimensions()
            width += w
            height = max(height, h)
        return width, height

    doLayout: (x, y) =>
        for child in @children
            child\setPosition(x, y)
            width, _ = child\getDimensions()
            x += width + padding

class FixedLayout extends Layout
    new: (width, height) =>
        @rect.width, @rect.height = width, height

    getDimensions: () =>
        return @rect.width, @rect.height

class Anchor extends Layout, Hierarchial
    new: (getBounds, calcPosition) =>
        @getBounds = getBounds
        @calcPosition = calcPosition

    doLayout: () =>
        r = @getBounds()
        for child in @children
            w, h = child\getDimensions()
            x, y = @calcPosition(r.width, r.height, w, h)
            child\setPosition(x + r.x, y + r.y)

anchor = ((fcn) -> ((parent) -> Anchor(() -> parent/getRect(), fcn)))

Anchors = {
    TopLeft: anchor (bounds_width, bounds_height, w, h) ->
        0, bounds_height
    TopRight: anchor (bounds_width, bounds_height, w, h) ->
        bounds_width - w, bh - h
    BottomLeft: anchor (bounds_width, bounds_height, w, h) ->
        0, 0
    BottomRight: anchor (bounds_width, bounds_height, w, h) ->
        bounds_width - w, 0
    Centered: anchor (bounds_width, bounds_height, w, h) ->
        (bounds_width - w) * 0.5, (bounds_height - h) * 0.5
    TopCenter: anchor (bounds_width, bounds_height, w, h) ->
        (bounds_width - w) * 0.5, bounds_height,
    BottomCenter: anchor (bounds_width, bounds_height, w, h) ->
        (bounds_width - w) * 0.5, 0
}


buttons = {
    "settings",
    "modules",
    "stuff"
}

params =
    padding: 2


button_layouts = [ FixedLayout(120, 24) for x in buttons ]

menu_container = HorizontalLayout(params)

menu_container\addAll(button_layouts)

menu_anchor = Anchors.TopCenter(window)
menu_anchor\addChild(menu_container)











































