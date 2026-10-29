"""Create small, project-owned matte and sky materials in Unreal Editor."""
import unreal

assets = unreal.AssetToolsHelpers.get_asset_tools()
library = unreal.MaterialEditingLibrary

def create(name, sky=False):
    path = "/Game/Materials/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.log("Preserving existing material: " + path)
        return
    material = assets.create_asset(name, "/Game/Materials", unreal.Material, unreal.MaterialFactoryNew())
    color = library.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -400, 0)
    color.set_editor_property("parameter_name", "Color")
    color.set_editor_property("default_value", unreal.LinearColor(.4, .5, .5, 1))
    if sky:
        material.set_editor_property("two_sided", True)
        material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        library.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    else:
        library.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
        roughness = library.create_material_expression(material, unreal.MaterialExpressionConstant, -400, 200)
        roughness.set_editor_property("r", .88)
        library.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
        # A subtle ambient term keeps stylized surfaces readable without Lumen.
        ambient = library.create_material_expression(material, unreal.MaterialExpressionMultiply, -150, 300)
        ambient.set_editor_property("const_b", .12)
        library.connect_material_expressions(color, "", ambient, "A")
        library.connect_material_property(ambient, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    library.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material):
        raise RuntimeError("Could not save " + path)
    unreal.log("Created " + path)

create("M_CourierMatte")
create("M_CourierSky", sky=True)
