dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "fox_idle_??.png"),
      sound_effect = {
         -- Sound files with numbers starting for 10 are generating silence. Remove when we move the sound triggering to programs
         directory = "sound/animals",
         name = "coyote",
      },
      hotspot = { 10, 13 },
      fps = 10,
   },
}
add_walking_animations(animations, dirname, "fox_walk", {11, 14}, 20)

world:new_critter_type{
   name = "fox",
   descname = _ "Fox",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
