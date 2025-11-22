import './hud.css'

export default function HUD({
  yaw,
  pitch,
  score,
  shots,
  targets,
  accuracy,
  status,
  onReset,
  onFire,
  aimHandlers,
}) {
  const degreesYaw = Math.round((yaw * 180) / Math.PI)
  const degreesPitch = Math.round((pitch * 180) / Math.PI)

  const shouldIgnorePointer = (event) =>
    event.target.closest('.hud__button, .chip') !== null

  const handlePointerDown = (event) => {
    if (shouldIgnorePointer(event)) return
    aimHandlers.onPointerDown(event)
  }

  const handlePointerMove = (event) => {
    if (shouldIgnorePointer(event)) return
    aimHandlers.onPointerMove(event)
  }

  return (
    <div
      className="hud"
      onPointerDown={handlePointerDown}
      onPointerMove={handlePointerMove}
      onPointerUp={aimHandlers.onPointerUp}
      onPointerLeave={aimHandlers.onPointerLeave}
      onWheel={aimHandlers.onWheel}
    >
      <div className="hud__top">
        <div className="hud__panel">
          <p className="label">Yaw</p>
          <p className="value">{degreesYaw}°</p>
        </div>
        <div className="hud__panel">
          <p className="label">Pitch</p>
          <p className="value">{degreesPitch}°</p>
        </div>
        <div className="hud__panel">
          <p className="label">Score</p>
          <p className="value">{score}</p>
        </div>
        <div className="hud__panel">
          <p className="label">Targets</p>
          <p className="value">{targets}</p>
        </div>
        <div className="hud__panel">
          <p className="label">Shots</p>
          <p className="value">{shots}</p>
        </div>
        <div className="hud__panel">
          <p className="label">Accuracy</p>
          <p className="value">{accuracy}</p>
        </div>
      </div>
      <div className="hud__status">{status}</div>
      <div className="hud__bottom">
        <button className="hud__button" onClick={onFire}>
          Fire
        </button>
        <button className="hud__button ghost" onClick={onReset}>
          Reset
        </button>
        <div className="hud__controls">
          <span className="chip">Space</span>
          <span className="chip">R</span>
          <span className="chip">Scroll</span>
          <span className="chip">Drag</span>
        </div>
      </div>
      <div className="hud__crosshair" />
    </div>
  )
}
