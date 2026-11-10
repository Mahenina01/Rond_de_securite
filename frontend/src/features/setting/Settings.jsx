import Profile from "./layout/Profile";
import System from "./layout/System";

const Settings = () => {
  return (
    <div className="flex min-w-0 flex-col items-stretch gap-4 xl:flex-row xl:gap-6">
      <Profile />
      <System />
    </div>
  );
};

export default Settings;
