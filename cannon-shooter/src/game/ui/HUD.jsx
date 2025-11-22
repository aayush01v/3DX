import './hud.css'

export default function HUD({ yaw, pitch, score, shots, targets, onReset, onFire, aimHandlers }) {
  const degreesYaw = Math.round((yaw * 180) / Math.PI)
  const degreesPitch = Math.round((pitch * 180) / Math.PI)

  return (
    <div
      className="hud"
      onPointerDown={aimHandlers.onPointerDown}
      onPointerMove={aimHandlers.onPointerMove}
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
      </div>
      <div className="hud__bottom">
        <button className="hud__button" onClick={onFire}>
          Fire
        </button>
        <button className="hud__button ghost" onClick={onReset}>
          Reset
        </button>
      </div>
      <div className="hud__crosshair" />
    </div>
  )
}
