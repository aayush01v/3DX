import { Link } from 'react-router-dom'
import './App.css'

function App() {
  return (
    <div className="app">
      <div className="card">
        <p className="eyebrow">Physics Playground</p>
        <h1>Cannon Shooter</h1>
        <p>
          Clear a field of glowing targets with a responsive cannon and real-time physics.
          Learn the controls below, then jump into the arena.
        </p>
        <ul className="feature-list">
          <li>Mouse to aim, scroll to adjust pitch</li>
          <li>Spacebar to fire, R to reset the course</li>
          <li>HUD with live stats, accuracy, and win state</li>
        </ul>
        <Link to="/cannon" className="primary">Launch the game</Link>
      </div>
    </div>
  )
}

export default App
