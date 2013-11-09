function love.load()
  love.graphics.setMode("400", "400", false, false, 0)
end

function love.draw()
  local x = love.mouse.getX()
  local y = love.mouse.getY()
  love.graphics.printf("mouse: ("..x..", "..y..")", 0, 200, 400, "center")
end

function love.keypressed(key, unicode)
  if key == 'escape' then
    love.event.push("quit")
  end
end
