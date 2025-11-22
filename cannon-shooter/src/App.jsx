import { Link } from 'react-router-dom'
import './App.css'

function App() {
  return (
    <div className="app">
      <div className="card">
        <h1>Cannon Shooter</h1>
        <p>Launch the 3D cannon game at the dedicated route.</p>
        <Link to="/cannon" className="primary">Open /cannon</Link>
      </div>
    </div>
  )
}

export default App
