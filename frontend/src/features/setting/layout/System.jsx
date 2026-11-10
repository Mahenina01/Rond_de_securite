import { useEffect, useRef, useState } from "react";
import Sun from "../../../assets/icons/sun.svg?react";
import Languages from "../../../assets/icons/langue.svg?react";
import ShieldUser from "../../../assets/icons/security.svg?react";
import Bell from "../../../assets/icons/alert-outlined.svg?react";
import ChevronRight from "../../../assets/icons/chevron-right.svg?react";
import Upload from "../../../assets/icons/upload.svg?react";
import LogoIcon from "../../../assets/icons/cardscan.svg";
import Modal from "../../../components/ui/Modal";
import Button from "../../../components/ui/Button";
import useLanguage from "../../../context/useLanguage";

const System = () => {
  const { language, setLanguage, t } = useLanguage();
  const [darkMode, setDarkMode] = useState(() =>
    document.documentElement.classList.contains("dark"),
  );
  const [notifications, setNotifications] = useState(true);
  const [isLanguageDialogOpen, setIsLanguageDialogOpen] = useState(false);
  const [logo, setLogo] = useState(null);
  const [logoName, setLogoName] = useState("");
  const fileInputRef = useRef(null);

  useEffect(() => {
    document.documentElement.classList.toggle("dark", darkMode);
    window.localStorage.setItem(
      "rond-security-theme",
      darkMode ? "dark" : "light",
    );
  }, [darkMode]);

  const handleLogoChange = (event) => {
    const file = event.target.files?.[0];

    if (!file) return;

    const extension = file.name.split(".").pop()?.toLowerCase();

    if (!["png", "svg"].includes(extension)) {
      alert(t("Veuillez sélectionner un fichier PNG ou SVG."));
      event.target.value = "";
      return;
    }

    if (file.size > 5 * 1024 * 1024) {
      alert(t("Le fichier ne doit pas dépasser 5 Mo."));
      event.target.value = "";
      return;
    }

    if (logo?.preview) {
      URL.revokeObjectURL(logo.preview);
    }

    setLogo({
      file,
      preview: URL.createObjectURL(file),
    });

    setLogoName(file.name);
    event.target.value = "";
  };

  return (
    <div className="flex w-full min-w-0 flex-col gap-4 xl:w-[40%] xl:shrink-0">
      {/* Paramètres système */}
      <section className="p-4 rounded-lg bg-surface">
        <h1 className="mb-6 font-sans text-xl font-semibold text-text-primary">
          {t("Paramètres systèmes")}
        </h1>

        <div className="flex flex-col gap-6">
          {/* Mode sombre */}
          <div className="flex items-center gap-3">
            <Sun className="w-6 h-6 shrink-0 text-text-primary" />

            <span className="flex-1 font-sans text-base font-normal text-text-primary">
              {t("Mode sombre")}
            </span>

            <span className="hidden text-xs text-text-secondary sm:block">
              {darkMode ? t("Activé") : t("Désactivé")}
            </span>

            <button
              type="button"
              role="switch"
              aria-checked={darkMode}
              aria-label={
                darkMode
                  ? t("Désactiver le mode sombre")
                  : t("Activer le mode sombre")
              }
              onClick={() => setDarkMode((value) => !value)}
              className={`relative h-6 w-12 shrink-0 rounded-full transition-colors ${
                darkMode ? "bg-primary" : "bg-gray-200"
              }`}
            >
              <span
                className={`absolute top-1 h-4 w-4 rounded-full bg-white shadow-sm transition-all ${
                  darkMode ? "left-7" : "left-1"
                }`}
              />
            </button>
          </div>

          {/* Langue */}
          <div className="flex items-center gap-3">
            <Languages className="w-6 h-6 shrink-0 text-text-primary" />

            <span className="flex-1 font-sans text-base font-normal text-text-primary">
              {t("Langue")}
            </span>

            <button
              type="button"
              onClick={() => setIsLanguageDialogOpen(true)}
              aria-label={t("Choisir la langue")}
              className="cursor-pointer bg-transparent text-sm text-text-primary hover:text-primary focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary"
            >
              {language === "fr" ? "Français" : "English"}
            </button>

            <ChevronRight className="w-5 h-5 text-gray-500 shrink-0" />
          </div>

          {/* Notifications */}
          <div className="flex items-center gap-3">
            <Bell className="w-6 h-6 shrink-0 text-text-primary" />

            <span className="flex-1 font-sans text-base font-normal text-text-primary">
              {t("Notifications")}
            </span>

            <span className="hidden text-xs text-text-secondary sm:block">
              {notifications ? t("Activées") : t("Désactivées")}
            </span>

            <button
              type="button"
              role="switch"
              aria-checked={notifications}
              aria-label={t("Activer les notifications")}
              onClick={() => setNotifications((value) => !value)}
              className={`relative h-6 w-12 shrink-0 rounded-full transition-colors ${
                notifications ? "bg-primary" : "bg-gray-200"
              }`}
            >
              <span
                className={`absolute top-1 h-4 w-4 rounded-full bg-white shadow-sm transition-all ${
                  notifications ? "left-7" : "left-1"
                }`}
              />
            </button>
          </div>

          {/* Sécurité */}
          <button
            type="button"
            onClick={() => alert("Ouvrir les paramètres de sécurité")}
            className="flex items-center w-full gap-3 text-left"
          >
            <ShieldUser className="w-6 h-6 shrink-0 text-text-primary" />

            <span className="flex-1 font-sans text-base font-normal text-text-primary">
              {t("Sécurité")}
            </span>

            <span className="px-3 py-2 text-xs font-semibold text-white bg-green-600 rounded-lg whitespace-nowrap">
              {t("2FA activé")}
            </span>

            <ChevronRight className="w-5 h-5 text-gray-500 shrink-0" />
          </button>
        </div>

        {/* Importation du logo */}
        <div className="mt-6">
          <input
            ref={fileInputRef}
            type="file"
            accept=".png,.svg,image/png,image/svg+xml"
            onChange={handleLogoChange}
            className="hidden"
          />

          <button
            type="button"
            onClick={() => fileInputRef.current?.click()}
            className="flex min-h-[180px] w-full flex-col items-center justify-center gap-3 rounded-lg border-2 border-dashed border-primary bg-gray-50/50 p-4 text-center transition hover:bg-primary/5"
          >
            <span className="font-sans text-lg font-semibold text-primary">
              {t("Logo")}
            </span>

            <span className="flex items-center justify-center w-16 h-16 overflow-hidden text-2xl font-bold text-white rounded-xl bg-primary">
              {logo ? (
                <img
                  src={logo.preview}
                  alt={t("Logo sélectionné")}
                  className="object-contain w-full h-full"
                />
              ) : (
                <img
                  src={LogoIcon}
                  alt={t("Logo actuel")}
                  className="object-contain w-full h-full"
                />
              )}
            </span>

            <span className="text-xs text-text-secondary">
              {logoName || "PNG / SVG"}
            </span>

            <span className="flex items-center gap-1 text-xs text-primary">
              <Upload className="h-3.5 w-3.5" />
              {t("Cliquer pour modifier")}
            </span>
          </button>
        </div>
      </section>

      {/* À propos */}
      <section className="p-4 rounded-lg bg-surface">
        <h2 className="mb-3 font-sans text-xl font-semibold text-text-primary">
          {t("À propos")}
        </h2>

        <p className="font-sans text-sm leading-6 text-text-secondary">
          {t(
            "Cette application web est conçue pour administrer et superviser les rondes de sécurité. Elle centralise les journaux d’activité (logs), assure le suivi des scans RFID aux points de contrôle et permet de consulter l’historique des passages. Elle facilite la détection des scans manquants, le suivi des agents et la traçabilité des opérations afin d’assurer une gestion fiable et efficace des rondes de surveillance.",
          )}
        </p>
      </section>

      <Modal
        isOpen={isLanguageDialogOpen}
        onClose={() => setIsLanguageDialogOpen(false)}
        title={t("Choisir la langue")}
        IconTitle={Languages}
        footer={
          <Button
            type="button"
            variant="outline"
            onClick={() => setIsLanguageDialogOpen(false)}
          >
            {t("Fermer")}
          </Button>
        }
      >
        <p className="mb-4 text-sm text-text-secondary">
          {t("Sélectionnez la langue de l’interface.")}
        </p>
        <div className="flex flex-col gap-2">
          {[
            { code: "fr", label: "Français" },
            { code: "en", label: "English" },
          ].map((option) => (
            <button
              key={option.code}
              type="button"
              aria-pressed={language === option.code}
              onClick={() => {
                setLanguage(option.code);
                setIsLanguageDialogOpen(false);
              }}
              className={`flex w-full items-center justify-between rounded-lg border px-4 py-3 text-left transition ${
                language === option.code
                  ? "border-primary bg-primary/5 text-primary"
                  : "border-gray-200 text-text-primary hover:bg-gray-50"
              }`}
            >
              <span>{option.label}</span>
              {language === option.code && (
                <span aria-hidden="true" className="text-sm font-semibold">
                  ✓
                </span>
              )}
            </button>
          ))}
        </div>
      </Modal>
    </div>
  );
};

export default System;
