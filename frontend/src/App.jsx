import { RouterProvider } from "react-router";
import router from "./routes/router";
import { Toaster } from "react-hot-toast";
import { LanguageProvider } from "./context/LanguageProvider";

export default function App() {
  return (
    <LanguageProvider>
      <Toaster position="top-center" />
      <RouterProvider router={router} />
    </LanguageProvider>
  );
}
