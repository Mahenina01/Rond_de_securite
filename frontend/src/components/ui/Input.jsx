import { useState } from "react";

import searchIcon from "../../assets/icons/search.svg";
import FilterIcon from "../../assets/icons/filter.svg?react";
import EyeIcon from "../../assets/icons/show-eye.svg?react";
import EyeOffIcon from "../../assets/icons/hide-eye.svg?react";
import useLanguage from "../../context/useLanguage";

const inputVariants = {
  default: {
    type: "text",
    placeholder: "Saisissez votre texte",
  },
  email: {
    type: "email",
    placeholder: "Entrez votre adresse e-mail",
  },
  search: {
    type: "search",
    placeholder: "Rechercher...",
  },
  searchFilter: {
    type: "search",
    placeholder: "Rechercher...",
  },
  password: {
    type: "password",
    placeholder: "Entrez votre mot de passe",
  },
};

const Input = ({
  variant = "default",
  label,
  name,
  value,
  onChange,
  onFilterClick,
  placeholder,
  required = false,
  className = "",
}) => {
  const [showPassword, setShowPassword] = useState(false);
  const { t } = useLanguage();

  const config = inputVariants[variant] ?? inputVariants.default;

  const isPassword = variant === "password";
  const isSearch = variant === "search" || variant === "searchFilter";
  const hasFilterButton = variant === "searchFilter";

  const inputType = isPassword && showPassword ? "text" : config.type;

  return (
    <div className="flex flex-col w-full gap-2">
      {label && (
        <label htmlFor={name} className="text-sm font-medium text-gray-700">
          {t(label)}
          {required && <span className="ml-1 text-red-500">*</span>}
        </label>
      )}

      <div className="relative flex items-center">
        {isSearch && (
          <img
            src={searchIcon}
            alt=""
            aria-hidden="true"
            className="absolute w-5 h-5 pointer-events-none left-3"
          />
        )}

        <input
          id={name}
          name={name}
          type={inputType}
          placeholder={t(placeholder ?? config.placeholder)}
          value={value}
          onChange={onChange}
          required={required}
          className={`
            w-full rounded-lg
            border border-text-muted
            bg-surface px-4 py-3 text-sm
            outline-none transition
            placeholder:text-text-muted
            focus:border-primary focus:ring-2 focus:ring-blue-100
            ${isSearch ? "pl-10" : ""}
            ${hasFilterButton ? "pr-12" : ""}
            ${isPassword ? "pr-12" : ""}
            ${className}
          `}
        />

        {hasFilterButton && (
          <button
            type="button"
            onClick={onFilterClick}
            disabled={!onFilterClick}
            aria-label={t("Ouvrir les filtres")}
            className="absolute flex items-center justify-center p-1 transition-colors rounded right-3 text-text-secondary hover:text-primary focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary disabled:cursor-not-allowed disabled:opacity-50"
          >
            <FilterIcon aria-hidden="true" className="w-5 h-5 text-primary" />
          </button>
        )}

        {isPassword && (
          <button
            type="button"
            onClick={() => setShowPassword((previous) => !previous)}
            aria-label={
              showPassword
                ? t("Masquer le mot de passe")
                : t("Afficher le mot de passe")
            }
            aria-pressed={showPassword}
            className="absolute flex items-center justify-center p-1 rounded right-3"
          >
            {showPassword ? (
              <EyeOffIcon className="w-7 h-7 text-text-muted" />
            ) : (
              <EyeIcon className="w-7 h-7 text-text-muted" />
            )}
          </button>
        )}
      </div>
    </div>
  );
};

export default Input;
