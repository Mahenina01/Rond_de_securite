import { useState } from "react";
import Input from "../../components/ui/Input";
import Button from "../../components/ui/Button";
import VerifyEmailIcon from "../../assets/icons/message-sent.svg?react";
import { useNavigate } from "react-router";

export default function VerifyCodePage() {
  const [code, setCode] = useState("");
  const [isSubmitting, setIsSubmitting] = useState(false);
  const navigate = useNavigate();

  const handleSubmit = (e) => {
    e.preventDefault();
    setIsSubmitting(true);

    setTimeout(() => {
      setIsSubmitting(false);
    }, 1500);
    //TODO : verification
    navigate("/reset-password");
  };
  const handleResend = () => {
    console.log("Demande de renvoi du code de vérification...");
    alert("Un nouveau code a été envoyé.");
  };
  return (
    <main className="flex items-center justify-center min-h-screen p-4 bg-background">
      <div className="w-[522px] bg-surface rounded-[32px] px-[30px] py-[40px] shadow-[0_4px_200px_-3px_rgba(0,0,0,0.25)] flex flex-col gap-[24px]">
        <div className="flex items-center justify-center py-2">
          <VerifyEmailIcon className="w-[212px] h-[171px] object-contain text-primary" />
        </div>

        <div className="flex flex-col gap-2 text-center">
          <h1 className="text-xl font-black tracking-tight text-text-primary">
            VÉRIFIEZ VOTRE ADRESSE E-MAIL
          </h1>
          <p className="text-sm leading-relaxed text-text-secondary">
            Nous avons envoyé un code de vérification à votre adresse e-mail.
            Saisissez le code reçu pour confirmer votre identité et continuer la
            réinitialisation de votre mot de passe.
          </p>
        </div>

        {/* Formulaire */}
        <form onSubmit={handleSubmit} className="flex flex-col gap-[24px]">
          {/* 3. Champ de saisie du code avec ton composant <Input /> */}
          <Input
            variant="default"
            label="Code de vérification"
            name="code"
            placeholder="code de vérification"
            value={code}
            onChange={(e) => setCode(e.target.value)}
            required={true}
          />

          {/* Ligne : "Vous n'avez pas reçu le code ? Renvoyer" */}
          <div className="text-sm text-text-secondary flex items-center gap-1.5 -mt-2">
            <span>Vous n'avez pas reçu le code ?</span>
            <button
              type="button"
              onClick={handleResend}
              className="font-semibold text-primary hover:underline focus:outline-none"
            >
              Renvoyer
            </button>
          </div>

          {/* 4. Section des boutons d'action */}
          <div className="flex items-center justify-between gap-4 pt-2">
            <Button
              type="button"
              variant="outline"
              className="justify-center"
              onClick={() => console.log("Retour en arrière")}
            >
              Retour
            </Button>
            <Button
              type="submit"
              variant="primary"
              state={isSubmitting ? "waiting" : "default"}
              disabled={!code || isSubmitting}
              className="justify-center"
            >
              {isSubmitting ? "Vérification..." : "Vérifier le code"}
            </Button>
          </div>
        </form>
      </div>
    </main>
  );
}
