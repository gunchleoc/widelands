dirname = path.dirname(__file__)

animations = {
   idle = {
      directory = dirname,
      basename = "idle",
      hotspot = { 8, 14 },
      fps = 20,
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {11, 21}, 25)


world:new_critter_type{
   name = "lynx",
   descname = _ "Lynx",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
