import LogOutIcon from "../../assets/icons/logout-outlined.svg?react";

const LogOutButton = ({ onclick }) => {
  return (
    <button
      className="flex flex-row items-center justify-center gap-2 p-2 border-2 rounded-[20px] border-danger hover:bg-danger-light"
      onClick={onclick}
    >
      <LogOutIcon className={"text-danger w-4 h-4"} />
      <p className="text-sm text-danger font-heading">Déconnexion</p>
    </button>
  );
};

export default LogOutButton;
