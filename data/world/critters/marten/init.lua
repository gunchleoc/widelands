dirname = path.dirname(__file__)

animations = {
   idle = {
      directory = dirname,
      basename ="idle",
      hotspot = { 11, 11 },
      fps = 20,
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {15, 14}, 50)

world:new_critter_type{
   name = "marten",
   descname = _ "Marten",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
