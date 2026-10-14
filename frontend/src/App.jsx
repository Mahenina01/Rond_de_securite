import AlertList from "./components/ui/AlertList";
import Button from "./components/ui/button";
function App() {
  return <main className="min-h-screen bg-background p-6">
  <div>
    <Button onClick={()=> alert("Action exécutée")}> primary</Button>
    <Button variant="disabled">disable</Button>
    <Button variant="outline">outTime</Button>
    <Button variant="waiting">waiting</Button>
  </div>
  <AlertList/>
  </main>;
}

export default App;
