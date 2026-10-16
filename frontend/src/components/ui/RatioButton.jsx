const RadioButton = ({ label, value, selected, onChange }) => {
  return (
    <label className="flex items-center gap-3 cursor-pointer">
      <input
        type="radio"
        name="format"
        value={value}
        checked={selected === value}
        onChange={() => onChange(value)}
        className="w-5 h-5 cursor-pointer accent-primary"
      />

      <span className="text-sm font-medium text-text-secondary">{label}</span>
    </label>
  );
};

export default RadioButton;
