run(function()
   -- Place 2 ships and test that the function will return the correct ones
   local p1 = game.players[1]
   local sea = map:get_field(6, 14)

   local ship1 = p1:place_ship(sea)
   assert_not_nil(ship1)
   assert_equal("ship", ship1.descr.type_name)

   local ship2 = p1:place_ship(sea)
   assert_not_nil(ship2)
   assert_equal("ship", ship2.descr.type_name)

   assert_not_equal(ship1.serial, ship2.serial)
   assert_not_equal(ship1.shipname, ship2.shipname)

   -- Don't place ships on land
   local land = map:get_field(10, 21)
   local ship1 = p1:place_ship(land)
   assert_nil(ship1)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
