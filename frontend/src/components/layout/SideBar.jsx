import IconButton from "../ui/IconButton";

const SideBar = () => {
  return (
    <aside className="w-64 p-4 space-y-2">
      <IconButton
        variant="dashboard"
        onClick={() => console.log("Tableau de bord")}
      />

      <IconButton variant="agents" onClick={() => console.log("Agents")} />

      <IconButton variant="device" onClick={() => console.log("Appareils")} />

      <IconButton
        variant="location"
        onClick={() => console.log("Emplacements")}
      />

      <IconButton
        variant="logs"
        onClick={() => console.log("Rapport d'audit")}
      />

      <IconButton variant="setting" onClick={() => console.log("Paramètres")} />
    </aside>
  );
};

export default SideBar;
