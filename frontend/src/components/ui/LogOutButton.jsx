import LogOutIcon from "../../assets/icons/logout-outlined.svg?react";

const LogOutButton = ({ onClick }) => {
  return (
    <button
      className="flex flex-row items-center justify-center w-full gap-3 px-4 py-3 border rounded-lg border-danger hover:bg-danger-light"
      onClick={onClick}
    >
      <LogOutIcon className={"text-danger w-5 h-5"} />
      <p className="text-sm text-danger font-heading">Déconnexion</p>
    </button>
  );
};

export default LogOutButton;
