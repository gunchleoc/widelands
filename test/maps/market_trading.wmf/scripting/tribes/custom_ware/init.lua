-- NOCOM anim doesn't work
dirname = "test/maps/market_trading.wmf/" .. path.dirname(__file__)

descriptions:new_ware_type {
   name = "custom_ware",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Custom Ware"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 6, 7 },
      },
   }
}
