"""Create original low-cost sunset materials and enable instanced terrain usage."""
import unreal

assets = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.MaterialEditingLibrary

def connect(source, output, target, pin):
    if not lib.connect_material_expressions(source, output, target, pin):
        raise RuntimeError("Cannot connect material pin: " + target.get_class().get_name() + "." + pin)

def rgb(hex_value):
    values = [int(hex_value[i:i+2], 16) / 255 for i in (0, 2, 4)]
    values = [x / 12.92 if x <= .04045 else ((x + .055) / 1.055) ** 2.4 for x in values]
    return unreal.LinearColor(*values, 1)

def material(name):
    path = "/Game/Materials/" + name
    obj = unreal.EditorAssetLibrary.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else assets.create_asset(name, "/Game/Materials", unreal.Material, unreal.MaterialFactoryNew())
    lib.delete_all_material_expressions(obj)
    obj.set_editor_property("two_sided", True)
    obj.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    return obj

def parameter(obj, name, value, x, y):
    expr = lib.create_material_expression(obj, unreal.MaterialExpressionVectorParameter, x, y)
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", rgb(value))
    return expr

def gradient(name, top, bottom, sky=False):
    obj = material(name)
    a = parameter(obj, "Horizon", bottom, -650, 0)
    b = parameter(obj, "Zenith", top, -650, 180)
    vector = lib.create_material_expression(obj, unreal.MaterialExpressionCameraVectorWS if sky else unreal.MaterialExpressionPixelNormalWS, -650, 380)
    mask = lib.create_material_expression(obj, unreal.MaterialExpressionComponentMask, -450, 380)
    for key in ("r", "g", "a"):
        mask.set_editor_property(key, False)
    mask.set_editor_property("b", True)
    connect(vector, "", mask, "")
    scale = lib.create_material_expression(obj, unreal.MaterialExpressionMultiply, -260, 380)
    scale.set_editor_property("const_b", -1.8 if sky else .5)
    connect(mask, "", scale, "A")
    bias = lib.create_material_expression(obj, unreal.MaterialExpressionAdd, -80, 380)
    bias.set_editor_property("const_b", .02 if sky else .5)
    connect(scale, "", bias, "A")
    clamp = lib.create_material_expression(obj, unreal.MaterialExpressionSaturate, 90, 380)
    connect(bias, "", clamp, "")
    lerp = lib.create_material_expression(obj, unreal.MaterialExpressionLinearInterpolate, 270, 80)
    connect(a, "", lerp, "A")
    connect(b, "", lerp, "B")
    connect(clamp, "", lerp, "Alpha")
    lib.connect_material_property(lerp, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(obj)
    unreal.EditorAssetLibrary.save_loaded_asset(obj)

gradient("M_CourierSkyGradient", "789BB9", "F3C6A5", True)
gradient("M_CourierCloud", "FFF1DE", "D6B6B9")
sun = material("M_CourierSun")
color = parameter(sun, "Color", "FFE4AF", -300, 0)
lib.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.recompile_material(sun)
unreal.EditorAssetLibrary.save_loaded_asset(sun)
terrain = unreal.EditorAssetLibrary.load_asset("/Game/Materials/M_CourierMatte")
terrain.set_editor_property("used_with_instanced_static_meshes", True)
lib.recompile_material(terrain)
unreal.EditorAssetLibrary.save_loaded_asset(terrain)
unreal.log("Courier sunset, cloud, sun and instanced terrain materials saved.")
