import { createBrowserRouter, Navigate } from "react-router";

import ProtectedRoute from "./protectedRoute";

import DashboardLayout from "../features/DashboardLayout";

import Dashboard from "../features/dashboard/Dashboard";
import Agents from "../features/agent/Agents";
import Devices from "../features/device/Devices";
import Reports from "../features/report/Reports";
import Settings from "../features/setting/Settings";
import SignIn from "../features/auth/SignIn";
import NotFound from "../features/notfound/NotFound";
import Locations from "../features/location/Locations";

import ForgotPasswordPage from "../features/auth/ForgotPasswordPage";
import ResetPasswordPage from "../features/auth/ResetPasswordPage";
import VerifyCodePage from "../features/auth/VerifyCodePage";
import PublicRoute from "./PublicRoute";

const router = createBrowserRouter([
  // Routes accessibles uniquement aux utilisateurs non authentifiés
  {
    element: <PublicRoute />,
    children: [
      {
        path: "/signin",
        element: <SignIn />,
      },
      {
        path: "/forgot-password",
        element: <ForgotPasswordPage />,
      },
      {
        path: "/verify-email",
        element: <VerifyCodePage />,
      },
      {
        path: "/reset-password",
        element: <ResetPasswordPage />,
      },
    ],
  },

  // Routes protégées
  {
    element: <ProtectedRoute />,
    children: [
      {
        path: "/",
        element: <DashboardLayout />,
        children: [
          {
            index: true,
            element: <Navigate to="/dashboard" replace />,
          },
          {
            path: "dashboard",
            element: <Dashboard />,
          },
          {
            path: "agents",
            element: <Agents />,
          },
          {
            path: "devices",
            element: <Devices />,
          },
          {
            path: "locations",
            element: <Locations />,
          },
          {
            path: "reports",
            element: <Reports />,
          },
          {
            path: "settings",
            element: <Settings />,
          },
        ],
      },
    ],
  },

  // Page 404
  {
    path: "*",
    element: <NotFound />,
  },
]);

export default router;
