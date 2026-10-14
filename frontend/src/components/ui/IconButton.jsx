import textVariant from "../../constants/textSideBar";

const IconButton = ({ variant = "dashboard", active = false, onClick }) => {
  const config = textVariant[variant];

  if (!config) return null;

  const Icon = config.icon;

  return (
    <button
      type="button"
      onClick={onClick}
      className={`flex w-full items-center gap-3 rounded-lg px-4 py-3 transition ${
        active
          ? "bg-blue-50 text-blue-600"
          : "text-text-secondary hover:bg-gray-100"
      }`}
    >
      <Icon className="w-5 h-5 shrink-0 text-text-secondary" />

      <span className="text-sm font-medium">{config.text}</span>
    </button>
  );
};

export default IconButton;
