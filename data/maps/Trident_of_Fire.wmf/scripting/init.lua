-- =======================================================================
--                      Scenario Trident of Fire
-- =======================================================================

include "scripting/coroutine.lua"
include "scripting/table.lua"
include "scripting/infrastructure.lua"
include "scripting/objective_utils.lua"
include "scripting/ui.lua"
include "scripting/set.lua"

game = wl.Game()
map = game.map

include "map:scripting/map_editing.lua"
include "map:scripting/initial_conditions.lua"

-- Field coordinates
p1_f_hq = map:get_field(116,37)
p1_f_port = map:get_field(63,53)
p1_f_vineyard = map:get_field(0,0)
p1_f_shipyard = map:get_field(115, 30)
p2_f_hq = map:get_field(44, 144)
p2_f_port = map:get_field(82,139)
p2_f_vineyard = map:get_field(47, 148)
p2_f_shipyard = map:get_field(48,138)
p3_f_hq = map:get_field(139, 101)
p3_f_port = map:get_field(101,93)
p3_f_vineyard = map:get_field(135,96)
p3_f_shipyard = map:get_field(143, 95)
p4_f_hq = map:get_field(28, 132)
p4_f_port = map:get_field(73,118)
p4_f_vineyard = map:get_field(25,135)
p4_f_shipyard = map:get_field(33, 134)
p5_f_hq = map:get_field(123, 11)
p5_f_port = map:get_field(78,27)
p5_f_vineyard = map:get_field(120,3)
p5_f_shipyard = map:get_field(125, 15)
p6_f_hq = map:get_field(11, 63)
p6_f_port = map:get_field(57,68)
p6_f_vineyard = map:get_field(11,54)
p6_f_shipyard = map:get_field(7, 67)

-- if true then
if map.player_slots[1].ai == "" then
   a = init_human_player(game.players[1], p1_f_hq, game.players[1].tribe_name)
else
   a = init_AI_player(game.players[1], p1_f_hq, p1_f_port, p1_f_vineyard, p1_f_shipyard, game.players[1].tribe_name)
end
if map.player_slots[2].ai == "" then
   a = init_human_player(game.players[2], p2_f_hq, game.players[2].tribe_name)
else
   a = init_AI_player(game.players[2], p2_f_hq, p2_f_port, p2_f_vineyard, p2_f_shipyard, game.players[2].tribe_name)
end
if map.player_slots[3].ai == ""  then
   a = init_AI_player(game.players[3], p3_f_hq, p3_f_port, p3_f_vineyard, p3_f_shipyard, game.players[3].tribe_name)
end

a = init_AI_player(game.players[4], p4_f_hq, p4_f_port, p4_f_vineyard, p4_f_shipyard, game.players[4].tribe_name)
a = init_AI_player(game.players[5], p5_f_hq, p5_f_port, p5_f_vineyard, p5_f_shipyard, game.players[5].tribe_name)
a = init_AI_player(game.players[6], p6_f_hq, p6_f_port, p6_f_vineyard, p6_f_shipyard, game.players[6].tribe_name)

run(flooding)
run(automatic_forester)
run(volcano_eruptions)
