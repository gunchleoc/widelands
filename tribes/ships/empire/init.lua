dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 115, 100 },
      fps = 10
   }
}
add_worker_animations(animations, "sail", dirname, "sail", {115, 100}, 10)


tribes:new_ship_type {
   name = "empire_ship",
   -- TRANSLATORS: This is the ship's name used in lists of units
   descname = _"Ship",
   capacity = 30,
   vision_range = 4,
   animations = animations,
}
