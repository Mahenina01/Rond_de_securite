import { useNavigate } from "react-router";
import Button from "../../components/ui/Button";
import useLanguage from "../../context/useLanguage";

const NotFound = () => {
  const navigate = useNavigate();
  const { t } = useLanguage();

  return (
    <main className="flex min-h-screen items-center justify-center bg-background px-4 py-12">
      <section
        aria-labelledby="not-found-title"
        className="w-full max-w-xl rounded-2xl border border-gray-200 bg-surface px-6 py-10 text-center shadow-sm sm:px-10 sm:py-14"
      >
        <div
          aria-hidden="true"
          className="mx-auto flex h-16 w-16 items-center justify-center rounded-full bg-primary/10 text-2xl font-bold text-primary"
        >
          !
        </div>
        <p className="mt-6 text-sm font-semibold uppercase tracking-[0.2em] text-primary">
          {t("Erreur 404")}
        </p>
        <h1
          id="not-found-title"
          className="mt-2 text-2xl font-bold text-text-primary sm:text-3xl"
        >
          {t("Page introuvable")}
        </h1>
        <p className="mx-auto mt-3 max-w-md text-sm leading-6 text-text-secondary sm:text-base">
          {t(
            "La page que vous recherchez n’existe pas ou a été déplacée. Vérifiez l’adresse ou retournez au tableau de bord.",
          )}
        </p>
        <div className="mt-8 flex flex-col justify-center gap-3 sm:flex-row">
          <Button type="button" onClick={() => navigate("/dashboard")}>
            {t("Retour au tableau de bord")}
          </Button>
          <Button
            type="button"
            variant="outline"
            onClick={() => navigate(-1)}
          >
            {t("Page précédente")}
          </Button>
        </div>
      </section>
    </main>
  );
};

export default NotFound;
